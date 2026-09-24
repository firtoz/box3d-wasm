//! Linear compute grids that preserve the existing one-dimensional fast path.
//! Keep the convention in sync with shaders/physics/dispatch.wgsl.

pub(crate) const LINEAR_DISPATCH_LIMIT: u32 = 65_535;
pub(crate) const LINEAR_DISPATCH_TILE: u32 = 256;

pub(crate) fn linear_dispatch_groups(groups: u32) -> [u32; 3] {
    if groups <= LINEAR_DISPATCH_LIMIT {
        [groups, 1, 1]
    } else {
        let rows = groups.div_ceil(LINEAR_DISPATCH_TILE);
        assert!(rows <= LINEAR_DISPATCH_LIMIT, "linear dispatch exceeds two-dimensional grid capacity");
        [LINEAR_DISPATCH_TILE, rows, 1]
    }
}

pub(crate) trait LinearDispatch {
    /// The shader must use the matching linear invocation/workgroup helpers.
    /// The last tiled row may contain padding; shader bounds checks still apply.
    fn dispatch_linear(&mut self, groups: u32);
}

impl LinearDispatch for wgpu::ComputePass<'_> {
    fn dispatch_linear(&mut self, groups: u32) {
        let [x, y, z] = linear_dispatch_groups(groups);
        self.dispatch_workgroups(x, y, z);
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn linear_dispatch_grid_boundaries_and_unique_coverage() {
        for groups in [0, 1, 255, 256, 65_534, 65_535, 65_536, 65_537, 131_072] {
            let [x, y, z] = linear_dispatch_groups(groups);
            assert!(x <= LINEAR_DISPATCH_LIMIT && y <= LINEAR_DISPATCH_LIMIT);
            assert_eq!(z, 1);
            if groups <= LINEAR_DISPATCH_LIMIT {
                assert_eq!([x, y, z], [groups, 1, 1]);
            }
            let mut seen = vec![false; groups as usize];
            for row in 0..y {
                for column in 0..x {
                    let index = column + row * LINEAR_DISPATCH_TILE;
                    if index < groups {
                        assert!(!seen[index as usize], "duplicate workgroup {index}");
                        seen[index as usize] = true;
                    }
                }
            }
            assert!(seen.into_iter().all(|v| v), "missing workgroup at {groups}");
        }
        assert_eq!(linear_dispatch_groups(256 * 65_535), [256, 65_535, 1]);
    }

    #[test]
    #[should_panic(expected = "two-dimensional grid capacity")]
    fn linear_dispatch_rejects_unrepresentable_grid() {
        linear_dispatch_groups(256 * 65_535 + 1);
    }
}
