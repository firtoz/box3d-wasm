//! Structural checks for captured GPU geometry. Not a convexity proof.
pub(super) fn mesh_tree(nodes:&[u32], triangles:usize) -> Result<(),String> {
    if nodes.len()%8!=0 {return Err("unaligned mesh tree".into());}
    let count=nodes.len()/8;if count==0 {return Ok(());}
    let mut seen=vec![false;count];let mut covered=vec![false;triangles];let mut stack=vec![0usize];
    while let Some(index)=stack.pop() {
        if index>=count || seen[index] {return Err("mesh tree child out of range or shared".into());}
        seen[index]=true;let node=&nodes[8*index..8*index+8];let data=node[3];
        if data&3==3 {
            let start=node[7] as usize;let length=(data>>2) as usize;
            let end=start.checked_add(length).ok_or("mesh leaf span overflow")?;
            if length==0 || end>triangles {return Err("mesh leaf triangle range invalid".into());}
            for value in &mut covered[start..end] {
                if *value {return Err("mesh triangle belongs to multiple leaves".into());}*value=true;
            }
        } else {
            let offset=(data>>2) as usize;
            if offset<=1 {return Err("mesh tree right child must follow left child".into());}
            stack.push(index.checked_add(offset).ok_or("mesh child overflow")?);stack.push(index+1);
        }
    }
    if seen.iter().any(|&v|!v) || covered.iter().any(|&v|!v) {return Err("unreachable mesh node or triangle".into());}
    Ok(())
}

pub(super) fn hull_topology(edges:&[u32], points:usize, planes:usize) -> Result<(),String> {
    if edges.len()%4!=0 {return Err("unaligned hull topology".into());}
    let count=edges.len()/4;
    for (index,edge) in edges.chunks_exact(4).enumerate() {
        let next=edge[0] as usize;let twin=edge[1] as usize;
        if next>=count || twin>=count || edge[2] as usize>=points || edge[3] as usize>=planes {
            return Err("hull topology reference out of range".into());
        }
        if twin==index || edges[4*twin+1] as usize!=index || edges[4*next+3]!=edge[3] {
            return Err("hull twin/face relationship invalid".into());
        }
        if edges[4*next+2]!=edges[4*twin+2] {return Err("hull edge endpoints disagree with twin".into());}
    }
    Ok(())
}

#[cfg(test)]
mod tests {
    use super::*;
    #[test]
    fn geometry_relationship_controls() {
        let mut nodes=vec![0;24];nodes[3]=2<<2;nodes[11]=(1<<2)|3;nodes[19]=(1<<2)|3;nodes[23]=1;
        mesh_tree(&nodes,2).unwrap();
        let mut bad=nodes.clone();bad[3]=3<<2;assert!(mesh_tree(&bad,2).is_err());
        let mut bad=nodes.clone();bad[23]=0;assert!(mesh_tree(&bad,2).is_err());
        let mut bad=nodes.clone();bad[19]=(2<<2)|3;assert!(mesh_tree(&bad,2).is_err());
        let edges=vec![1,5,0,0, 2,4,1,0, 0,3,2,0, 4,2,0,1, 5,1,2,1, 3,0,1,1];
        hull_topology(&edges,3,2).unwrap();
        let mut bad=edges.clone();bad[0]=6;assert!(hull_topology(&bad,3,2).is_err());
        let mut bad=edges.clone();bad[1]=4;assert!(hull_topology(&bad,3,2).is_err());
        let mut bad=edges.clone();bad[3]=2;assert!(hull_topology(&bad,3,2).is_err());
    }
}
