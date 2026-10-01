#!/usr/bin/env python3
"""Record an immutable native scene protocol; captures are never timing trials."""
import argparse, hashlib, json, os, signal, subprocess, time
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1] if Path(__file__).parent.name == 'scripts' else Path.cwd()
def sha(p): return hashlib.sha256(Path(p).read_bytes()).hexdigest()
def write(p, data): Path(p).write_text(json.dumps(data, indent=2) + '\n')

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--protocol', type=Path, required=True)
    ap.add_argument('--kind', choices=['cpu', 'gpu'], required=True)
    a = ap.parse_args()
    protocol_bytes = a.protocol.read_bytes()
    p = json.loads(protocol_bytes)
    assert p['purpose'] == 'precommit visual/health evidence, not performance'
    assert p['budget'] == {'cpu': 6, 'gpu': 6, 'timing': 0, 'retries': 0}
    assert sha(__file__) == p['recorder_sha256']
    for n, h in p['source_files'].items(): assert sha(ROOT / n) == h, n
    for n, h in p['assets'].items(): assert sha(ROOT / n) == h, n
    out = ROOT / p['artifact_directory']
    state_path = out / 'receipt.json'
    state = json.loads(state_path.read_text()) if state_path.exists() else {'status': 'ready', 'results': [], 'protocol_sha256': sha(a.protocol)}
    assert state['protocol_sha256'] == sha(a.protocol)
    assert state['status'] in ['ready', 'cpu-complete'], state['status']
    assert not any(r['kind'] == a.kind for r in state['results']), 'No reruns'
    assert a.kind == 'cpu' or len(state['results']) == 6
    viewer = p['viewers'][a.kind]
    binary = ROOT / viewer['binary']
    assert sha(binary) == viewer['binary_sha256']
    assert sha(ROOT / viewer['receipt']) == viewer['receipt_sha256']
    build = json.loads((ROOT / viewer['receipt']).read_text())
    assert build['status'] == 'built' and build['inputs_before'] == build['inputs_after']
    for n, h in build['inputs_before'].items():
        if n not in p['build_applicability_exclusions']: assert sha(ROOT / n) == h, n
    for u in build['compiled_units']:
        assert sha(u['file']) == u['source_sha256']
        assert sha(u['object']) == u['object_sha256']
    env = {k: v for k, v in os.environ.items() if not k.startswith(('GPU_', 'WGPU_', 'VK_', '__NV', '__GLX', 'LIBGL'))}
    env.update(p['environments'][a.kind])
    display = env['DISPLAY']
    assert not Path('/tmp/.X' + display[1:] + '-lock').exists()
    xlog = (out / (a.kind + '-xvfb.log')).open('w')
    server = subprocess.Popen([str(ROOT / p['xvfb']), display, '-screen', '0', '1280x720x24', '-nolisten', 'tcp', '-ac'], stdout=xlog, stderr=subprocess.STDOUT, start_new_session=True)
    app = ff = None
    try:
        for _ in range(100):
            if subprocess.run(['xdpyinfo', '-display', display], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL).returncode == 0: break
            assert server.poll() is None, 'Xvfb failed'
            time.sleep(.1)
        else: raise RuntimeError('Xvfb initialization timeout')
        for scene in p['scenes']:
            result = out / a.kind / scene['id']; result.mkdir(parents=True, exist_ok=False)
            working = result / 'cwd'; working.mkdir()
            (working / 'data').symlink_to(ROOT.parents[1] / 'box3d/data', target_is_directory=True)
            # Empty settings preserve all defaults but suppress the first-run help/replay redirect.
            (working / 'settings.ini').write_text('{}\n')
            video = ROOT / 'recordings/snapshots' / viewer['label'] / (scene['id'] + '.mp4')
            video.parent.mkdir(parents=True, exist_ok=True)
            assert not video.exists(), 'Never overwrite clips'
            cmd = [str(binary), '--sample-name', scene['name'], '--bench-json', str(result / 'health.json'), '--warmup', '0', '--timed', '300', '--paced', '--completed-step', '--health-scan', '--hide-ui']
            capture = ['ffmpeg', '-hide_banner', '-loglevel', 'warning', '-f', 'x11grab', '-draw_mouse', '0', '-framerate', '30', '-video_size', '1280x720', '-i', display, '-c:v', 'libx264', '-preset', 'veryfast', '-crf', '20', '-pix_fmt', 'yuv420p', '-movflags', '+faststart', str(video)]
            launch = {'command': cmd, 'capture_command': capture, 'environment': p['environments'][a.kind], 'working_directory': str(working), 'initial_settings': '{}\n', 'binary_sha256': sha(binary), 'purpose': p['purpose'], 'protocol_sha256': sha(a.protocol)}
            write(result / 'launch.json', launch)
            row = {'kind': a.kind, 'scene': scene['id'], 'status': 'running'}
            state['results'].append(row); state.update(status='running', running_scene=scene['id'], running_kind=a.kind, driver_pid=os.getpid()); write(state_path, state)
            with (result / 'ffmpeg.log').open('w') as flog, (result / 'stdout').open('w') as stdout, (result / 'stderr').open('w') as stderr:
                ff = subprocess.Popen(capture, stdin=subprocess.PIPE, stdout=flog, stderr=subprocess.STDOUT, env=env, start_new_session=True)
                time.sleep(.5)
                app = subprocess.Popen(cmd, cwd=working, env=env, stdout=stdout, stderr=stderr, start_new_session=True)
                launch['app_pid'] = app.pid; launch['ffmpeg_pid'] = ff.pid; write(result / 'launch.json', launch)
                try: code = app.wait(timeout=p['watchdog_seconds_per_scene'])
                except subprocess.TimeoutExpired:
                    os.killpg(app.pid, signal.SIGTERM); app.wait(timeout=30); row['status'] = 'timeout'; raise
                ff.communicate(b'q\n', timeout=30)
            probe = json.loads(subprocess.check_output(['ffprobe', '-v', 'error', '-show_streams', '-of', 'json', str(video)], text=True))
            stream = next(s for s in probe['streams'] if s['codec_type'] == 'video')
            row.update(exit=code, ffmpeg_exit=ff.returncode, video_sha256=sha(video), encoded_frames=int(stream['nb_frames']), duration_seconds=float(stream['duration']))
            write(result / 'capture.json', {**row, 'probe': probe, 'includes_startup_loading': True})
            assert code == 0 and ff.returncode == 0, row
            assert stream['width'] == 1280 and stream['height'] == 720 and stream['r_frame_rate'] == '30/1' and int(stream['nb_frames']) >= 300
            health = json.loads((result / 'health.json').read_text())
            assert health['status'] == 'ok' and health['mode'] == viewer.get('health_mode', a.kind) and health['frames_observed'] == health['measured'] == 300
            assert health['enable_sleep'] and health['completed_step_mode'] and not health['unpaced']
            assert health['last_submitted_step'] == health['last_completed_step'] == health['last_rendered_pose'] == 300 and health['in_flight'] == 0 and not health['gpu_fail']
            assert not health['village_drop'], 'Preserve upstream setup'
            assert len(health['frames']) == 300
            for i, frame in enumerate(health['frames'], 1):
                assert frame['submitted_step'] == frame['completed_step'] == frame['rendered_pose'] == i
                assert frame['nan_count'] == 0 and not frame['gpu_contact_metrics']['capacity_loss']
            row.update(status='pass', health_sha256=sha(result / 'health.json'))
            write(state_path, state)
            print(a.kind, scene['id'], 'capture/health pass', flush=True)
            app = ff = None
        state['status'] = 'cpu-complete' if a.kind == 'cpu' else 'captured'
    except BaseException as e:
        state.update(status='stopped', error=str(e), unlaunched=[{'kind': k, 'scene': s['id']} for k in ['cpu', 'gpu'] for s in p['scenes'] if not any(r['kind'] == k and r['scene'] == s['id'] for r in state['results'])])
        write(state_path, state)
        raise
    finally:
        if app is not None and app.poll() is None: os.killpg(app.pid, signal.SIGTERM); app.wait(timeout=30)
        if ff is not None and ff.poll() is None: ff.communicate(b'q\n', timeout=30)
        server.terminate(); server.wait(timeout=30); xlog.close()
        for k in ['driver_pid', 'running_scene', 'running_kind']: state.pop(k, None)
        write(state_path, state)
    assert a.protocol.read_bytes() == protocol_bytes

if __name__ == '__main__': main()
