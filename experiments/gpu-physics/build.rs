fn main() {
    let arch = std::env::var("CARGO_CFG_TARGET_ARCH").unwrap_or_default();
    if arch == "wasm32" {
        return;
    }
    let manifest = std::path::PathBuf::from(std::env::var("CARGO_MANIFEST_DIR").unwrap());
    let box3d_include = manifest.join("../../box3d/include");
    let box3d_src = manifest.join("../../box3d/src");
    if std::env::var_os("CARGO_FEATURE_EXTERNAL_C_SHIM").is_none() {
    cc::Build::new()
        .file(manifest.join("c_abi/shim.c"))
        .include(&box3d_include)
        .warnings(false)
        .compile("gpu_box3d_c_abi");
    }
    // Host-side hull factories (CreateCylinder/CreateHull/CloneAndTransform).
    // Same C as the CPU engine; Rust only uploads the cooked blob to GPU buffers.
    cc::Build::new()
        .files([
            box3d_src.join("hull.c"),
            box3d_src.join("core.c"),
            box3d_src.join("math_functions.c"),
            box3d_src.join("distance.c"),
        ])
        .include(&box3d_include)
        .include(&box3d_src)
        .warnings(false)
        .compile("gpu_box3d_hull_cook");
    println!("cargo:rerun-if-changed=c_abi/shim.c");
    println!("cargo:rerun-if-changed=c_abi/native_clock.h");
    println!("cargo:rerun-if-changed=c_abi/growable_slots.h");
    println!("cargo:rerun-if-changed=c_abi/compound_mesh_bake.h");
    println!("cargo:rerun-if-changed=c_abi/compound_mesh_instances.h");
    println!("cargo:rerun-if-changed={}", box3d_src.join("hull.c").display());
    println!("cargo:rerun-if-changed={}", box3d_src.join("core.c").display());
    println!("cargo:rerun-if-changed={}", box3d_src.join("math_functions.c").display());
    println!("cargo:rerun-if-changed={}", box3d_src.join("distance.c").display());
    for rel in [
        "shaders/physics/types.wgsl",
        "shaders/physics/math.wgsl",
        "shaders/physics/rotation.wgsl",
        "shaders/physics/hull.wgsl",
        "shaders/physics/collide.wgsl",
        "shaders/physics/broadphase.wgsl",
        "shaders/physics/solve.wgsl",
        "shaders/physics/integrate.wgsl",
        "shaders/render.wgsl",
    ] {
        println!("cargo:rerun-if-changed={rel}");
    }
}
