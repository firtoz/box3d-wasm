# Recording and Replay shared controls

Seven shared call sites reviewed on actual CPU and four GPU routes. [Acceptance, preserved failed harness subsets and protocol deviation](recording-replay-acceptance.json). [UI08 filename](UI08.md), [UI09 native-player exclusion](UI09.md).

CPU Recording collapses/reopens; actual filename input changes; Record Now keeps the world and seed step; Record (restart) creates a new world at step0. Both Stop actions save nonempty files29,906 and28,432bytes. Ten physics steps produce more recording/display frames (actual endpoint123), so transport checks use the actual UI endpoint. Actual CPU valid-file load, forward1/2, back1, endpoint123, home0, play and pause pass. Native GTK cancel preserves the world on all5routes.

GPU sidebar explicitly reports Recording unavailable and hides file/record/stop actions on all4routes. Valid CPU recording bytes are rejected by the excluded native-player API; UI09 records the misleading modal. This is reviewed unsupported behavior, not native recording/player qualification. Remaining CPU Replay widgets/query fixtures and sample-specific controls stay open.

Only one28KBrecording is retained for remaining CPU Replay controls. Temporary short-path dialog fixtures are deleted in each app finally block. No new product fixes or engine builds.
