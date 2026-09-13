# Aggregate layout repair for the GPU experiment

An isolated, in-process SPIR-V transformation for the experiment's pinned Naga
output. Explicit-layout aggregates reached through Function, Private or Workgroup
pointers receive separate logical types; buffer interfaces retain their original
layout. Logical copies bridge whole-value loads and stores. Unsupported constructs
return errors. This is not a general validator or support claim for arbitrary
SPIR-V producers.

`repair(&[u32])` accepts logical-addressing SPIR-V 1.4–1.6 and returns words. The
CLI takes input/output SPIR-V paths and also checks idempotence. No subprocess or
external assembler is used by the library. Vulkan validation remains an independent
test responsibility.

Run `cargo test --release --manifest-path compiler/spirv-layout/Cargo.toml` from the
experiment directory. Tests cover buffer declarations, arithmetic preservation,
composites, idempotence and rejected inputs. The offline shader corpus and isolated
Vulkan backend hook are documented in `../../artifacts/spirv-layout-rust/README.md`.
This crate is not enabled by the ordinary engine build.
