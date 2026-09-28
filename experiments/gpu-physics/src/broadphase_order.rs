// SPDX-FileCopyrightText: 2025 Erin Catto
// SPDX-License-Identifier: MIT
// Adapted from Box3D dynamic_tree.c for deterministic contact creation
// priority. This computes ordering metadata only;
// collision detection and simulation remain on the GPU.

#[derive(Clone, Copy, Debug, PartialEq)]
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
    enlarged: bool,
}
#[derive(Default)]
struct Tree {
    nodes: Vec<Node>,
    root: Option<usize>,
    free: Vec<usize>,
}
impl Tree {
    fn allocate(&mut self, node: Node) -> usize {
        if let Some(index) = self.free.pop() {
            self.nodes[index] = node;
            index
        } else {
            let index = self.nodes.len();
            self.nodes.push(node);
            index
        }
    }

    fn enlarge(&mut self, leaf: usize, bounds: Bounds) {
        assert!(self.nodes[leaf].children.is_none());
        self.nodes[leaf].bounds = bounds;
        let mut cursor = self.nodes[leaf].parent;
        while let Some(index) = cursor {
            let node = &mut self.nodes[index];
            let enlarged = node.bounds.union(bounds);
            let changed = node.bounds != enlarged;
            node.bounds = enlarged;
            node.enlarged = true;
            cursor = node.parent;
            if !changed {
                break;
            }
        }
        while let Some(index) = cursor {
            let node = &mut self.nodes[index];
            if node.enlarged {
                break;
            }
            node.enlarged = true;
            cursor = node.parent;
        }
    }

    fn partition(&self, leaves: &mut [usize]) -> usize {
        if leaves.len() <= 2 {
            return leaves.len() / 2;
        }
        let mut lower = self.nodes[leaves[0]].bounds.center();
        let mut upper = lower;
        for &leaf in &leaves[1..] {
            let center = self.nodes[leaf].bounds.center();
            for axis in 0..3 {
                lower[axis] = lower[axis].min(center[axis]);
                upper[axis] = upper[axis].max(center[axis]);
            }
        }
        let d: [f32; 3] = std::array::from_fn(|i| upper[i] - lower[i]);
        let axis = if d[0] >= d[1] && d[0] >= d[2] {
            0
        } else if d[1] >= d[2] {
            1
        } else {
            2
        };
        let pivot = 0.5 * (lower[axis] + upper[axis]);
        let mut first = 0;
        let mut last = leaves.len();
        while first < last {
            while first < last && self.nodes[leaves[first]].bounds.center()[axis] < pivot {
                first += 1;
            }
            while first < last && self.nodes[leaves[last - 1]].bounds.center()[axis] >= pivot {
                last -= 1;
            }
            if first < last {
                leaves.swap(first, last - 1);
                first += 1;
                last -= 1;
            }
        }
        if first > 0 && first < leaves.len() {
            first
        } else {
            leaves.len() / 2
        }
    }

    fn build(&mut self, leaves: &mut [usize]) -> usize {
        if leaves.len() == 1 {
            return leaves[0];
        }
        let split = self.partition(leaves);
        let index = self.allocate(Node {
            bounds: self.nodes[leaves[0]].bounds,
            parent: None,
            children: None,
            shape: usize::MAX,
            enlarged: false,
        });
        let (left, right) = leaves.split_at_mut(split);
        let a = self.build(left);
        let b = self.build(right);
        self.nodes[index].children = Some([a, b]);
        self.nodes[a].parent = Some(index);
        self.nodes[b].parent = Some(index);
        self.refit(index);
        index
    }

    fn rebuild(&mut self, full: bool) {
        let Some(root) = self.root else {
            return;
        };
        let mut stack = vec![root];
        let mut leaves = Vec::new();
        while let Some(index) = stack.pop() {
            let node = &mut self.nodes[index];
            if node.children.is_none() || (!node.enlarged && !full) {
                node.parent = None;
                leaves.push(index);
            } else {
                let [a, b] = node.children.unwrap();
                stack.push(b);
                stack.push(a);
                self.free.push(index);
            }
        }
        self.root = Some(self.build(&mut leaves));
    }

    fn reversed_query_order(&self) -> Vec<usize> {
        let mut stack: Vec<_> = self.root.into_iter().collect();
        let mut order = Vec::new();
        while let Some(index) = stack.pop() {
            if let Some([a, b]) = self.nodes[index].children {
                stack.push(b);
                stack.push(a);
            } else {
                order.push(self.nodes[index].shape);
            }
        }
        order
    }
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
    fn insert(&mut self, shape: usize, bounds: Bounds) -> usize {
        let leaf = self.allocate(Node {
            bounds,
            parent: None,
            children: None,
            shape,
            enlarged: false,
        });
        let Some(_) = self.root else {
            self.root = Some(leaf);
            return leaf;
        };
        let sibling = self.sibling(bounds);
        let old_parent = self.nodes[sibling].parent;
        let parent = self.allocate(Node {
            bounds: bounds.union(self.nodes[sibling].bounds),
            parent: old_parent,
            children: Some([sibling, leaf]),
            shape: usize::MAX,
            enlarged: false,
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
        leaf
    }
}

/// Leaf order after reversing Box3D's query callback list (which prepends pairs).
/// Inputs must follow proxy creation order and contain fat world-space bounds.
pub(crate) fn initial_leaf_order(proxies: impl IntoIterator<Item = (usize, Bounds)>) -> Vec<usize> {
    let mut tree = Tree::default();
    for (shape, bounds) in proxies {
        tree.insert(shape, bounds);
    }
    tree.reversed_query_order()
}

/// Initial topology and workspace for contact_order.wgsl. Bounds come from the
/// engine's own scene; native capture data is used only by regression tests.
pub(crate) fn gpu_tree_seed(
    proxies: impl IntoIterator<Item = (usize, Bounds)>,
    node_capacity: usize,
) -> Vec<u32> {
    let mut tree = Tree::default();
    for (shape,bounds) in proxies { tree.insert(shape,bounds); }
    assert!(tree.nodes.len() <= node_capacity);
    let mut words = vec![0; 8 + 19*node_capacity];
    words[0] = tree.root.map_or(u32::MAX, |i| i as u32);
    words[1] = tree.nodes.len() as u32;
    words[2] = u32::MAX;
    words[3] = tree.nodes.iter().filter(|n| n.children.is_none()).count() as u32;
    words[4] = node_capacity as u32;
    for (index,node) in tree.nodes.iter().enumerate() {
        let out = &mut words[8+12*index..8+12*(index+1)];
        for axis in 0..3 {
            out[axis]=node.bounds.lower[axis].to_bits();
            out[3+axis]=node.bounds.upper[axis].to_bits();
        }
        out[6]=node.parent.map_or(u32::MAX, |i| i as u32);
        let children=node.children.map_or([u32::MAX;2], |v| v.map(|i| i as u32));
        out[7..9].copy_from_slice(&children);
        out[9]=node.shape as u32;
        out[10]=u32::from(node.enlarged);
        out[11]=u32::MAX;
    }
    words
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn enlarged_tree_order_matches_native_partial_rebuilds() {
        // Captured with independent native linker wrappers, with the simulation
        // prefix verified unchanged. No oracle ordering is used at runtime.
        let mut tree = Tree::default();
        let mut proxies = std::collections::HashMap::new();
        let mut queries = 0;
        let mut enlargements = 0;
        let mut rebuilds = 0;
        // Versioned little-endian records: initial/enlarge (shape + six f32
        // bounds), rebuild (full flag), query (frame + native leaf sequence).
        let fixture = include_bytes!("fixtures/broadphase_tree_updates.bin");
        assert_eq!(&fixture[..8], b"B3TO\x01\0\0\0");
        let mut words = fixture[8..]
            .chunks_exact(4)
            .map(|b| u32::from_le_bytes(b.try_into().unwrap()));
        while let Some(kind) = words.next() {
            match kind {
                0 | 1 => {
                    let shape = words.next().unwrap() as usize;
                    let lower = std::array::from_fn(|_| f32::from_bits(words.next().unwrap()));
                    let upper = std::array::from_fn(|_| f32::from_bits(words.next().unwrap()));
                    let bounds = Bounds { lower, upper };
                    if kind == 0 {
                        assert!(proxies.insert(shape, tree.insert(shape, bounds)).is_none());
                    } else {
                        tree.enlarge(proxies[&shape], bounds);
                        enlargements += 1;
                    }
                }
                2 => {
                    tree.rebuild(words.next().unwrap() != 0);
                    rebuilds += 1;
                }
                3 => {
                    let frame = words.next().unwrap();
                    let count = words.next().unwrap() as usize;
                    let mut expected: Vec<_> =
                        words.by_ref().take(count).map(|v| v as usize).collect();
                    assert_eq!(expected.len(), count);
                    expected.reverse();
                    assert_eq!(tree.reversed_query_order(), expected, "frame {frame}");
                    queries += 1;
                }
                other => panic!("unexpected tree event {other}"),
            }
        }
        assert_eq!((queries, enlargements, rebuilds), (167, 17282, 165));
        assert_eq!(tree.nodes.len(), 2 * proxies.len() - 1);
        assert!(tree.free.is_empty());
    }

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
