// SPDX-FileCopyrightText: 2025 Erin Catto
// SPDX-License-Identifier: MIT
// Adapted from Box3D dynamic_tree.c: insertion-only tree for deterministic
// initial contact creation priority. This computes ordering metadata only;
// collision detection and simulation remain on the GPU.

#[derive(Clone, Copy, Debug)]
pub(crate) struct Bounds {
    pub lower: [f32; 3],
    pub upper: [f32; 3],
}
impl Bounds {
    fn union(self, other: Self) -> Self {
        Self {
            lower: std::array::from_fn(|i| self.lower[i].min(other.lower[i])),
            upper: std::array::from_fn(|i| self.upper[i].max(other.upper[i])),
        }
    }
    fn area(self) -> f32 {
        let [x, y, z] = std::array::from_fn(|i| self.upper[i] - self.lower[i]);
        2.0 * (x * z + y * x + z * y)
    }
    fn center(self) -> [f32; 3] {
        std::array::from_fn(|i| 0.5 * (self.lower[i] + self.upper[i]))
    }
}
#[derive(Clone, Debug)]
struct Node {
    bounds: Bounds,
    parent: Option<usize>,
    children: Option<[usize; 2]>,
    shape: usize,
}
#[derive(Default)]
struct Tree {
    nodes: Vec<Node>,
    root: Option<usize>,
}
impl Tree {
    fn sibling(&self, bounds: Bounds) -> usize {
        let root = self.root.unwrap();
        let mut index = root;
        let mut base = self.nodes[root].bounds.area();
        let mut direct = self.nodes[root].bounds.union(bounds).area();
        let mut inherited = 0.0;
        let mut best = root;
        let mut best_cost = direct;
        while let Some(children) = self.nodes[index].children {
            let cost = direct + inherited;
            if cost < best_cost {
                best = index;
                best_cost = cost;
            }
            inherited += direct - base;
            let mut lower = [f32::MAX; 2];
            let mut child_area = [0.0; 2];
            let mut child_direct = [0.0; 2];
            let mut leaf = [false; 2];
            for k in 0..2 {
                let child = &self.nodes[children[k]];
                leaf[k] = child.children.is_none();
                child_direct[k] = child.bounds.union(bounds).area();
                if leaf[k] {
                    let cost = child_direct[k] + inherited;
                    if cost < best_cost {
                        best = children[k];
                        best_cost = cost;
                    }
                } else {
                    child_area[k] = child.bounds.area();
                    lower[k] =
                        inherited + child_direct[k] + (bounds.area() - child_area[k]).min(0.0);
                }
            }
            if leaf[0] && leaf[1] {
                break;
            }
            if best_cost <= lower[0] && best_cost <= lower[1] {
                break;
            }
            if lower[0] == lower[1] && !leaf[0] {
                let center = bounds.center();
                for k in 0..2 {
                    let d: [f32; 3] = std::array::from_fn(|i| {
                        self.nodes[children[k]].bounds.center()[i] - center[i]
                    });
                    lower[k] = d[0] * d[0] + d[1] * d[1] + d[2] * d[2];
                }
            }
            let k = if lower[0] < lower[1] && !leaf[0] {
                0
            } else {
                1
            };
            debug_assert!(!leaf[k]);
            index = children[k];
            base = child_area[k];
            direct = child_direct[k];
        }
        best
    }
    fn refit(&mut self, i: usize) {
        if let Some([a, b]) = self.nodes[i].children {
            self.nodes[i].bounds = self.nodes[a].bounds.union(self.nodes[b].bounds);
        }
    }
    fn swap_grandchild(&mut self, root: usize, branch: usize, child: usize) {
        let children = self.nodes[root].children.unwrap();
        let leaf = children[branch];
        let inner = children[1 - branch];
        let replacement = self.nodes[inner].children.unwrap()[child];
        self.nodes[root].children.as_mut().unwrap()[branch] = replacement;
        self.nodes[inner].children.as_mut().unwrap()[child] = leaf;
        self.nodes[leaf].parent = Some(inner);
        self.nodes[replacement].parent = Some(root);
        self.refit(inner);
        self.refit(root);
    }
    fn rotate(&mut self, root: usize) {
        let Some([b, c]) = self.nodes[root].children else {
            return;
        };
        let leaf_b = self.nodes[b].children.is_none();
        let leaf_c = self.nodes[c].children.is_none();
        if leaf_b && leaf_c {
            return;
        }
        if leaf_b != leaf_c {
            let (branch, leaf, inner) = if leaf_b { (0, b, c) } else { (1, c, b) };
            let [f, g] = self.nodes[inner].children.unwrap();
            let base = self.nodes[inner].bounds.area();
            let cost_f = self.nodes[leaf].bounds.union(self.nodes[g].bounds).area();
            let cost_g = self.nodes[leaf].bounds.union(self.nodes[f].bounds).area();
            if base < cost_f && base < cost_g {
                return;
            }
            self.swap_grandchild(root, branch, if cost_f < cost_g { 0 } else { 1 });
        } else {
            let [d, e] = self.nodes[b].children.unwrap();
            let [f, g] = self.nodes[c].children.unwrap();
            let area_b = self.nodes[b].bounds.area();
            let area_c = self.nodes[c].bounds.area();
            let costs = [
                area_b + self.nodes[b].bounds.union(self.nodes[g].bounds).area(),
                area_b + self.nodes[b].bounds.union(self.nodes[f].bounds).area(),
                area_c + self.nodes[c].bounds.union(self.nodes[e].bounds).area(),
                area_c + self.nodes[c].bounds.union(self.nodes[d].bounds).area(),
            ];
            let mut cost = area_b + area_c;
            let mut choice = None;
            for (i, candidate) in costs.into_iter().enumerate() {
                if candidate < cost {
                    cost = candidate;
                    choice = Some(i);
                }
            }
            if let Some(i) = choice {
                self.swap_grandchild(root, i / 2, i % 2);
            }
        }
    }
    fn insert(&mut self, shape: usize, bounds: Bounds) {
        let leaf = self.nodes.len();
        self.nodes.push(Node {
            bounds,
            parent: None,
            children: None,
            shape,
        });
        let Some(_) = self.root else {
            self.root = Some(leaf);
            return;
        };
        let sibling = self.sibling(bounds);
        let old_parent = self.nodes[sibling].parent;
        let parent = self.nodes.len();
        self.nodes.push(Node {
            bounds: bounds.union(self.nodes[sibling].bounds),
            parent: old_parent,
            children: Some([sibling, leaf]),
            shape: usize::MAX,
        });
        if let Some(old) = old_parent {
            let children = self.nodes[old].children.as_mut().unwrap();
            let k = if children[0] == sibling { 0 } else { 1 };
            children[k] = parent;
        } else {
            self.root = Some(parent);
        }
        self.nodes[sibling].parent = Some(parent);
        self.nodes[leaf].parent = Some(parent);
        let mut cursor = Some(parent);
        while let Some(i) = cursor {
            self.refit(i);
            self.rotate(i);
            cursor = self.nodes[i].parent;
        }
    }
}

/// Leaf order after reversing Box3D's query callback list (which prepends pairs).
/// Inputs must follow proxy creation order and contain fat world-space bounds.
pub(crate) fn initial_leaf_order(proxies: impl IntoIterator<Item = (usize, Bounds)>) -> Vec<usize> {
    let mut tree = Tree::default();
    for (shape, bounds) in proxies {
        tree.insert(shape, bounds);
    }
    let mut stack: Vec<_> = tree.root.into_iter().collect();
    let mut order = Vec::new();
    while let Some(i) = stack.pop() {
        if let Some([a, b]) = tree.nodes[i].children {
            stack.push(b);
            stack.push(a);
        } else {
            order.push(tree.nodes[i].shape);
        }
    }
    order
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn initial_tree_order_matches_box3d_rotations_and_ties() {
        // Independent b3DynamicTree_CreateProxy + Query results, reversed to
        // match the prepended broadphase pair list. Includes symmetric costs,
        // duplicate centers, and both leaf/internal and internal/internal rotations.
        let centers = [
            [0., 0., 0.],
            [2., 0., 0.],
            [-2., 0., 0.],
            [0., 3., 0.],
            [0., -3., 0.],
            [0., 0., 4.],
            [0., 0., -4.],
            [5., 5., 5.],
            [5., 5., 5.],
            [-5., -5., -5.],
            [1., 1., 1.],
            [-1., -1., -1.],
        ];
        for expected in [
            vec![0, 1, 2],
            vec![0, 1, 2, 5, 3, 4],
            vec![0, 1, 2, 3, 4, 5, 6, 7, 8],
            vec![3, 4, 11, 0, 1, 2, 10, 5, 6, 9, 7, 8],
        ] {
            let proxies = centers[..expected.len()].iter().enumerate().map(|(i, c)| {
                let radius = 0.25f32 + 0.1f32 * (i % 3) as f32;
                (
                    i,
                    Bounds {
                        lower: c.map(|x| x - radius),
                        upper: c.map(|x| x + radius),
                    },
                )
            });
            assert_eq!(initial_leaf_order(proxies), expected);
        }
        assert!(initial_leaf_order([]).is_empty());
    }
}
