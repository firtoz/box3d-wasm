// Native contact API/lifetime contract, including compound public-pair gaps.
#include "box3d/box3d.h"
#include "box3d/collision.h"
#include <cstdio>
#include <cmath>
#include <cstring>
#include <atomic>
#include <initializer_list>
static bool same(b3ContactId a, b3ContactId b) {
  return a.index1 == b.index1 && a.world0 == b.world0 && a.generation == b.generation;
}
#define REQUIRE(condition, code) do { if (!(condition)) { std::printf("failure=%s code=%d\n", #condition, code); return code; } } while (0)
static bool sameShape(b3ShapeId a, b3ShapeId b) {
  return a.index1 == b.index1 && a.world0 == b.world0 && a.generation == b.generation;
}
// Exercise the public factories through the exact same GPU shim link order as
// native samples. AABB-only stand-ins must fail before any clone dereference.
static int checkFactories() {
  b3BoxHull boxes[] = {b3MakeBoxHull(.5f, 1, 2), b3MakeCubeHull(.5f),
                      b3MakeOffsetBoxHull(.5f, 1, 2, {3, 4, 5})};
  for (int i = 0; i < 3; ++i) {
    const auto& h = boxes[i].base;
    REQUIRE(h.version == B3_HULL_VERSION && h.byteCount == sizeof(b3BoxHull), 40);
    REQUIRE(h.vertexCount == 8 && h.edgeCount == 24 && h.faceCount == 6, 41);
    REQUIRE(h.pointOffset > 0 && h.pointOffset + 8 * sizeof(b3Vec3) <= h.byteCount, 42);
    REQUIRE(h.planeOffset > 0 && h.planeOffset + 6 * sizeof(b3Plane) <= h.byteCount, 43);
    REQUIRE(h.hash != 0 && std::fabs(h.volume - (i == 1 ? 1.0f : 8.0f)) < 1e-6f, 44);
    auto points = b3GetHullPoints(&h);
    for (int v = 0; v < 8; ++v) {
      REQUIRE(points[v].x == h.aabb.lowerBound.x || points[v].x == h.aabb.upperBound.x, 45);
      REQUIRE(points[v].y == h.aabb.lowerBound.y || points[v].y == h.aabb.upperBound.y, 46);
      REQUIRE(points[v].z == h.aabb.lowerBound.z || points[v].z == h.aabb.upperBound.z, 47);
    }
    b3Transform translation = {{7, -2, 3}, b3Quat_identity};
    auto clone = b3CloneAndTransformHull(&h, translation, {1, 1, 1});
    REQUIRE(clone && clone->byteCount == h.byteCount, 48);
    auto clonedPoints = b3GetHullPoints(clone);
    for (int v = 0; v < 8; ++v) {
      REQUIRE(std::fabs(clonedPoints[v].x - points[v].x - 7) < 1e-6f &&
              std::fabs(clonedPoints[v].y - points[v].y + 2) < 1e-6f &&
              std::fabs(clonedPoints[v].z - points[v].z - 3) < 1e-6f, 49);
    }
    b3DestroyHull(clone);
  }
  std::puts("hull-factories=pass");
  return 0;
}
static std::atomic<int> filterCalls{0}, preCalls{0};
static bool vetoFirst = false;
static bool acceptFilter(b3ShapeId, b3ShapeId, void*) { ++filterCalls; return true; }
static bool acceptPre(b3ShapeId, b3ShapeId, b3Pos, b3Vec3, void*) {
  int ordinal = preCalls.fetch_add(1); return !vetoFirst || ordinal != 0;
}
static int checkTopology(bool reuse, bool checkEvents = false, bool checkPersistence = false, int transition = 0, bool compound = false) {
  auto wd = b3DefaultWorldDef(); wd.gravity = b3Vec3_zero; wd.enableSleep = false;
  auto world = b3CreateWorld(&wd);
  auto gd = b3DefaultBodyDef(); auto ground = b3CreateBody(world, &gd);
  auto sd = b3DefaultShapeDef(); sd.enableContactEvents = true;
  auto unusedHull = b3MakeOffsetBoxHull(.2f,.3f,.4f,{100,100,100});
  auto unused = b3CreateHullShape(ground, &sd, &unusedHull.base);
  auto floorHull = b3MakeBoxHull(4,.5f,4);
  b3CompoundData* compoundData=nullptr;
  b3ShapeId floor{};
  if (compound) {
    b3CompoundHullDef hulls[2]{};
    hulls[0].hull=&unusedHull.base; hulls[1].hull=&floorHull.base;
    for(auto& h:hulls) { h.transform={b3Vec3_zero,b3Quat_identity}; h.material=b3DefaultSurfaceMaterial(); }
    b3CompoundDef def{}; def.hulls=hulls; def.hullCount=2;
    compoundData=b3CreateCompound(&def);
    REQUIRE(compoundData,85);
    floor=b3CreateBakedCompoundShape(ground,&sd,compoundData);
  } else {
    floor=b3CreateHullShape(ground,&sd,&floorHull.base);
  }
  auto bd = b3DefaultBodyDef(); bd.type = b3_dynamicBody; bd.position = {0,1,0};
  auto body = b3CreateBody(world, &bd);
  auto box = b3MakeBoxHull(.5f,.5f,.5f);
  auto shape = b3CreateHullShape(body, &sd, &box.base);
  b3World_Step(world,1.0f/60,4);
  b3ContactData data[4]{};
  REQUIRE(b3Body_GetContactData(body,data,4)==1,70);
  auto original = data[0].contactId;
  REQUIRE(b3Contact_IsValid(original),71);
  if (reuse) {
    b3DestroyShape(shape,true);
    shape = b3CreateHullShape(body,&sd,&box.base);
  } else {
    b3DestroyShape(unused,false);
  }
  if (transition==1) {
    auto spare=b3DefaultBodyDef();
    for(int i=0;i<300;++i) b3CreateBody(world,&spare);
  } else if (transition==5) {
    auto query=[&]() { return b3World_CastRayClosest(world,{0,4,0},{0,-8,0},b3DefaultQueryFilter()).hit; };
    REQUIRE(query(),86);
    // Reuse the earlier shape slot, then grow between two query rebuilds.
    b3CreateHullShape(ground,&sd,&unusedHull.base);
    REQUIRE(query(),87);
    auto spare=b3DefaultBodyDef();
    for(int i=0;i<300;++i) b3CreateBody(world,&spare);
    REQUIRE(query(),88);
    b3World_Step(world,0,4);
  } else if (transition==2) {
    b3World_Step(world,0,4);
  } else if (transition==3 || transition==4) {
    auto ray=b3World_CastRayClosest(world,{0,4,0},{0,-8,0},b3DefaultQueryFilter());
    REQUIRE(ray.hit,81);
    if (transition==4) {
      REQUIRE(b3Body_GetContactData(body,data,4)==1,82);
      REQUIRE(same(data[0].contactId,original),83);
      REQUIRE(sameShape(data[0].shapeIdA,floor) && sameShape(data[0].shapeIdB,shape),84);
    }
  }
  // Deliberately skip reads so adjacency cannot hide replacement GPU roots.
  for (int i=0;i<((checkEvents || checkPersistence) ? 1 : 3);++i) b3World_Step(world,1.0f/60,4);
  if (checkEvents) {
    auto events=b3World_GetContactEvents(world);
    std::printf("topology begins=%d ends=%d\n",events.beginCount,events.endCount);
    REQUIRE(events.beginCount==0 && events.endCount==0,78);
  }
  REQUIRE(b3Body_GetContactData(body,data,4)==1,72);
  std::printf("topology reuse=%d old=%d/%d new=%d/%d\n",int(reuse),original.index1,
    original.generation,data[0].contactId.index1,data[0].contactId.generation);
  REQUIRE(same(original,data[0].contactId) == !reuse,73);
  REQUIRE(b3Contact_IsValid(original) == !reuse,74);
  REQUIRE(sameShape(data[0].shapeIdA,floor) && sameShape(data[0].shapeIdB,shape),75);
  REQUIRE(data[0].manifoldCount==1 && data[0].manifolds[0].normal.y>.99f,76);
  if (checkPersistence) {
    const auto& m=data[0].manifolds[0];
    REQUIRE(m.pointCount==4,79);
    for(int i=0;i<m.pointCount;++i) {
      std::printf("topology point=%d persisted=%d\n",i,int(m.points[i].persisted));
      REQUIRE(m.points[i].persisted == !reuse,80);
    }
  }
  auto v=b3Body_GetLinearVelocity(body);
  REQUIRE(std::fabs(v.x)<.001f && std::fabs(v.y)<.001f && std::fabs(v.z)<.001f,77);
  b3DestroyWorld(world);
  if(compoundData) b3DestroyCompound(compoundData);
  std::puts("contact-lifetime=pass"); return 0;
}
static int checkSpeculative(int mode) {
  auto wd=b3DefaultWorldDef();wd.gravity=b3Vec3_zero;wd.enableSleep=false;auto w=b3CreateWorld(&wd);
  auto gd=b3DefaultBodyDef();auto g=b3CreateBody(w,&gd);
  b3Vec3 v[]={{-4,0,-4},{-4,0,4},{4,0,4},{4,0,-4}};int ix[]={0,1,2,2,3,0};
  b3MeshDef md{};md.vertices=v;md.vertexCount=4;md.indices=ix;md.triangleCount=2;md.identifyEdges=true;
  auto mesh=b3CreateMesh(&md,nullptr,0);REQUIRE(mesh,89);
  auto sd=b3DefaultShapeDef();sd.enableSpeculativeContact=mode!=2;
  b3CreateMeshShape(g,&sd,mesh,{1,1,1});
  auto bd=b3DefaultBodyDef();bd.type=b3_dynamicBody;bd.position={0,.51f,0};auto b=b3CreateBody(w,&bd);
  sd.enableSpeculativeContact=mode!=1 && mode!=3;
  auto box=b3MakeBoxHull(.5f,.5f,.5f);b3Sphere sphere={b3Vec3_zero,.5f};
  if(mode==3) b3CreateSphereShape(b,&sd,&sphere); else b3CreateHullShape(b,&sd,&box.base);
  b3World_Step(w,1.f/60,4);b3ContactData d[8]{};int n=b3Body_GetContactData(b,d,8);
  std::printf("speculative mode=%d contacts=%d\n",mode,n);
  REQUIRE(n==((mode==1 || mode==2) ? 0 : 1),90);
  if(mode==3) {
    int points=0;
    for(int i=0;i<n;++i) for(int j=0;j<d[i].manifoldCount;++j) points+=d[i].manifolds[j].pointCount;
    REQUIRE(points==1,91);
  }
  b3DestroyWorld(w);b3DestroyMesh(mesh);
  std::puts("contact-lifetime=pass");return 0;
}
static int checkCompoundMeshMaterial(bool compound, bool remap = false, int material = 1) {
  auto wd=b3DefaultWorldDef();wd.enableSleep=false;auto world=b3CreateWorld(&wd);
  auto gd=b3DefaultBodyDef();auto ground=b3CreateBody(world,&gd);
  b3Vec3 vertices[]={{-10,0,-30},{-10,0,-10},{10,0,-20},{-10,0,10},{-10,0,30},{10,0,20}};
  int indices[]={0,1,2,3,4,5};uint8_t materialIndices[]={0,1};
  b3MeshDef md{};md.vertices=vertices;md.vertexCount=6;md.indices=indices;md.triangleCount=2;
  md.materialIndices=materialIndices;md.identifyEdges=true;
  auto mesh=b3CreateMesh(&md,nullptr,0);REQUIRE(mesh && mesh->materialCount==2,92);
  b3SurfaceMaterial materials[]={b3DefaultSurfaceMaterial(),b3DefaultSurfaceMaterial()};
  materials[0].friction=0;materials[0].userMaterialId=101;
  materials[1].friction=1;materials[1].userMaterialId=202;
  auto sd=b3DefaultShapeDef();sd.baseMaterial.friction=0;
  b3CompoundData* data=nullptr;
  if(compound) {
    b3CompoundMeshDef child{};child.meshData=mesh;child.transform=b3Transform_identity;
    child.scale=b3Vec3_one;child.materials=materials;child.materialCount=2;
    b3CompoundDef def{};def.meshes=&child;def.meshCount=1;
    auto small=b3MakeCubeHull(.25f);b3CompoundHullDef preceding{};
    preceding.hull=&small.base;preceding.transform={{100,100,100},b3Quat_identity};
    preceding.material=materials[1];
    if(remap) { def.hulls=&preceding;def.hullCount=1; }
    data=b3CreateCompound(&def);REQUIRE(data,93);
    b3CreateBakedCompoundShape(ground,&sd,data);
  } else {
    sd.materials=materials;sd.materialCount=2;b3CreateMeshShape(ground,&sd,mesh,b3Vec3_one);
  }
  auto bd=b3DefaultBodyDef();bd.type=b3_dynamicBody;bd.position={-2,.51f,material==1 ? 20.f : -20.f};bd.linearVelocity={1,0,0};
  auto body=b3CreateBody(world,&bd);auto box=b3MakeBoxHull(.5f,.5f,.5f);
  sd=b3DefaultShapeDef();sd.baseMaterial.friction=1;b3CreateHullShape(body,&sd,&box.base);
  for(int i=0;i<120;++i)b3World_Step(world,1.f/60,4);
  auto v=b3Body_GetLinearVelocity(body);auto pos=b3Body_GetPosition(body);
  std::printf("compound=%d remap=%d material=%d vx=%g y=%g\n",int(compound),int(remap),material,v.x,pos.y);
  REQUIRE(std::fabs(v.x-(material==1 ? 0.f : 1.f))<.05f && std::fabs(pos.y-.5f)<.02f,94);
  b3DestroyWorld(world);if(data)b3DestroyCompound(data);b3DestroyMesh(mesh);
  std::puts("contact-lifetime=pass");return 0;
}
int main(int argc, char** argv) {
  if(argc==2 && std::strcmp(argv[1],"compound-mesh-remapped-high")==0) return checkCompoundMeshMaterial(true,true,1);
  if(argc==2 && std::strcmp(argv[1],"compound-mesh-remapped-low")==0) return checkCompoundMeshMaterial(true,true,0);

  if(argc==2 && std::strcmp(argv[1],"compound-mesh-material")==0) return checkCompoundMeshMaterial(true);
  if(argc==2 && std::strcmp(argv[1],"mesh-material-control")==0) return checkCompoundMeshMaterial(false);

  if(argc==2 && std::strcmp(argv[1],"mesh-speculative-on")==0) return checkSpeculative(0);
  if(argc==2 && std::strcmp(argv[1],"mesh-speculative-convex-off")==0) return checkSpeculative(1);
  if(argc==2 && std::strcmp(argv[1],"mesh-speculative-mesh-off")==0) return checkSpeculative(2);
  if(argc==2 && std::strcmp(argv[1],"mesh-speculative-sphere-off")==0) return checkSpeculative(3);

  if (argc==2 && std::strcmp(argv[1],"shape-hole-repeated")==0) return checkTopology(false,true,true,5);
  if (argc==2 && std::strcmp(argv[1],"shape-hole-compound")==0) return checkTopology(false,false,true,0,true);
  if (argc==2 && std::strcmp(argv[1],"shape-hole-grow")==0) return checkTopology(false,false,true,1);
  if (argc==2 && std::strcmp(argv[1],"shape-hole-zero")==0) return checkTopology(false,false,true,2);
  if (argc==2 && std::strcmp(argv[1],"shape-hole-query-data")==0) return checkTopology(false,false,true,4);
  if (argc==2 && std::strcmp(argv[1],"shape-hole-query")==0) return checkTopology(false,false,true,3);
  if (argc==2 && std::strcmp(argv[1],"shape-reuse-history")==0) return checkTopology(true,false,true);

  if (argc==2 && std::strcmp(argv[1],"shape-hole-persistence")==0) return checkTopology(false,false,true);
  if (argc==2 && std::strcmp(argv[1],"shape-hole-events")==0) return checkTopology(false,true);
  if (argc==2 && std::strcmp(argv[1],"shape-hole")==0) return checkTopology(false);
  if (argc==2 && std::strcmp(argv[1],"shape-reuse")==0) return checkTopology(true);
  if (argc == 2 && std::strcmp(argv[1], "factories") == 0) return checkFactories();
  bool sphere = argc == 2 && std::strcmp(argv[1], "sphere") == 0;
  bool initialGap = argc == 2 && std::strcmp(argv[1], "compound-initial-gap") == 0;
  bool unread = argc == 2 && std::strcmp(argv[1], "compound-unread") == 0;
  bool queries = argc == 2 && std::strcmp(argv[1], "compound-queries") == 0;
  vetoFirst = argc == 2 && std::strcmp(argv[1], "compound-veto") == 0;
  bool historyCallbacks = argc == 2 && std::strcmp(argv[1], "compound-fat-batched-callback") == 0;
  bool callbacks = vetoFirst || (argc == 2 && std::strcmp(argv[1], "compound-callbacks") == 0);
  bool overlap = callbacks || (argc == 2 && std::strcmp(argv[1], "compound-overlap") == 0);
  bool growBatched = argc == 2 && std::strcmp(argv[1], "compound-fat-grow-batched") == 0;
  bool zeroFat = argc == 2 && std::strcmp(argv[1], "compound-fat-batched-zero") == 0;
  bool batchedFat = growBatched || zeroFat || historyCallbacks || (argc == 2 && std::strcmp(argv[1], "compound-fat-batched") == 0);
  bool materialFat = argc == 2 && std::strcmp(argv[1], "compound-fat-material") == 0;
  bool growFat = argc == 2 && std::strcmp(argv[1], "compound-fat-grow") == 0;
  bool fat = growFat || materialFat || batchedFat || (argc == 2 && std::strcmp(argv[1], "compound-fat") == 0);
  bool compound = fat || queries || overlap || initialGap || unread || (argc == 2 && std::strcmp(argv[1], "compound") == 0);
  if (argc > 2 || (argc == 2 && !sphere && !compound)) return 64;
  auto wd = b3DefaultWorldDef(); wd.gravity = b3Vec3_zero; wd.enableSleep = false;
  auto world = b3CreateWorld(&wd);
  if (callbacks || historyCallbacks) {
    b3World_SetCustomFilterCallback(world, acceptFilter, nullptr);
    b3World_SetPreSolveCallback(world, acceptPre, nullptr);
  }
  auto gd = b3DefaultBodyDef(); auto ground = b3CreateBody(world, &gd);
  auto sd = b3DefaultShapeDef(); sd.enableContactEvents = true;
  sd.enableCustomFiltering = callbacks || historyCallbacks;
  sd.enablePreSolveEvents = callbacks || historyCallbacks;
  auto floor = b3MakeBoxHull(compound ? .5f : 4, .5f, compound ? .5f : 4);
  b3CompoundData* compoundData = nullptr;
  b3ShapeId groundShape{};
  if (compound) {
    b3CompoundHullDef hulls[2]{};
    for (int i=0; i<2; ++i) {
      hulls[i].hull = &floor.base;
      hulls[i].transform = {{i == 0 || overlap ? -3.0f : 3.0f, 0, 0}, b3Quat_identity};
      hulls[i].material = b3DefaultSurfaceMaterial();
    }
    b3CompoundDef definition{}; definition.hulls = hulls; definition.hullCount = 2;
    compoundData = b3CreateCompound(&definition);
    REQUIRE(compoundData, 34);
    groundShape = b3CreateBakedCompoundShape(ground, &sd, compoundData);
  } else {
    groundShape = b3CreateHullShape(ground, &sd, &floor.base);
  }
  auto bd = b3DefaultBodyDef(); bd.type = b3_dynamicBody; bd.position = {compound ? -3.0f : 0.0f,1,0};
  auto body = b3CreateBody(world, &bd); auto box = b3MakeBoxHull(.5f,.5f,.5f);
  b3Sphere ball = {{0,0,0}, 0.5f};
  auto shape = sphere ? b3CreateSphereShape(body, &sd, &ball) : b3CreateHullShape(body, &sd, &box.base);
  if (initialGap) b3Body_SetTransform(body, {0,1,0}, b3Quat_identity);
  b3World_Step(world, 1.0f/60, 4);
  if (initialGap) {
    b3ContactData gap[2]{};
    // CPU traverses child bounds before creating a new compound contact.
    REQUIRE(b3Body_GetContactCapacity(body) == 0, 50);
    REQUIRE(b3Body_GetContactData(body, gap, 2) == 0, 51);
    b3Body_SetTransform(body, {-3,1,0}, b3Quat_identity);
    b3World_Step(world, 1.0f/60, 4);
  }
  REQUIRE(b3Body_GetContactCapacity(body) >= 1, 10);
  REQUIRE(b3Shape_GetContactCapacity(shape) >= 1, 11);
  b3ContactData data[2]{};
  if (overlap) {
    // contact.c keys compounds by parent, other shape, AND child index.
    int touching = vetoFirst ? 1 : 2;
    REQUIRE(b3Body_GetContactCapacity(body) >= 2, 63);
    REQUIRE(b3Body_GetContactData(body, data, 2) == touching, 56);
    if (!vetoFirst) REQUIRE(!same(data[0].contactId, data[1].contactId), 57);
    for (int i = 0; i < touching; ++i) {
      auto& entry = data[i];
      REQUIRE(entry.manifoldCount == 1 && sameShape(entry.shapeIdA, groundShape) && sameShape(entry.shapeIdB, shape), 58);
      REQUIRE(b3Contact_IsValid(entry.contactId), 59);
    }
    auto events = b3World_GetContactEvents(world);
    REQUIRE(events.beginCount == touching, 60);
    if (callbacks) {
      std::printf("filter_calls=%d pre_calls=%d\n", filterCalls.load(), preCalls.load());
      REQUIRE(filterCalls == 2 && preCalls == 2, 62);
    }
    b3DestroyWorld(world); b3DestroyCompound(compoundData);
    std::puts("contact-lifetime=pass"); return 0;
  }
  REQUIRE(b3Body_GetContactData(body, data, 2) == 1, 12);
  auto id = data[0].contactId;
  REQUIRE(sameShape(data[0].shapeIdA, groundShape) && sameShape(data[0].shapeIdB, shape), 32);
  REQUIRE(data[0].manifolds && data[0].manifolds[0].normal.y > 0.99f, 33);
  REQUIRE(b3Contact_IsValid(id), 13);
  REQUIRE(data[0].manifoldCount == 1 && data[0].manifolds && data[0].manifolds[0].pointCount == (sphere ? 1 : 4), 14);
  REQUIRE(b3Shape_GetContactData(shape, data, 2) == 1 && same(id, data[0].contactId), 15);
  if (compound) {
    auto gapRay = b3World_CastRayClosest(world, {0, 4, 0}, {0, -8, 0}, b3DefaultQueryFilter());
    REQUIRE(!gapRay.hit, 54); // The parent AABB is not collision geometry.
    auto childRay = b3World_CastRayClosest(world, {3, 4, 0}, {0, -8, 0}, b3DefaultQueryFilter());
    REQUIRE(childRay.hit && sameShape(childRay.shapeId, groundShape) && childRay.childIndex == 1, 55);
    if (queries) {
      b3DestroyWorld(world); b3DestroyCompound(compoundData);
      std::puts("contact-lifetime=pass"); return 0;
    }
  }
  auto event = b3World_GetContactEvents(world);
  REQUIRE(event.beginCount == 1 && same(event.beginEvents[0].contactId, id), 16);
  auto direct = b3Contact_GetData(id);
  REQUIRE(same(direct.contactId, id) && direct.manifoldCount == 1 && direct.manifolds, 17);
  REQUIRE(b3Body_GetContactData(body, nullptr, 0) == 0, 18);
  REQUIRE(b3Shape_GetContactData(shape, nullptr, 0) == 0, 19);
  REQUIRE(std::fabs(b3World_GetContactRecycleDistance(world) - 0.05f) < 1e-7f, 30);
  b3World_SetContactRecycleDistance(world, -1.0f);
  REQUIRE(b3World_GetContactRecycleDistance(world) == 0.0f, 31);
  b3Body_SetTransform(body, {0,compound ? 1.0f : 1.04f,0}, b3Quat_identity);
  b3World_Step(world, 1.0f/60, 4);
  REQUIRE(b3Contact_IsValid(id), 20);
  direct = b3Contact_GetData(id);
  if (direct.manifoldCount) std::printf("ghost_points=%d sep=%g y=%g\n",direct.manifolds[0].pointCount,direct.manifolds[0].points[0].separation,b3Body_GetPosition(body).y);
  std::printf("ghost_manifolds=%d capacity=%d\n", direct.manifoldCount, b3Body_GetContactCapacity(body));
  REQUIRE(direct.manifoldCount == 0 && direct.manifolds == nullptr, 21);
  REQUIRE(b3Body_GetContactCapacity(body) >= 1 && b3Shape_GetContactCapacity(shape) >= 1, 22);
  REQUIRE(b3Body_GetContactData(body, data, 2) == 0 && b3Shape_GetContactData(shape, data, 2) == 0, 23);
  event = b3World_GetContactEvents(world);
  REQUIRE(event.endCount == 1 && same(event.endEvents[0].contactId, id), 24);
  if (growBatched) {
    // Reallocation must preserve both bounds and their consumed-command epoch.
    // The next batch must replay x=4.10 before x=4.14, not alias the old batch.
    auto spare = b3DefaultBodyDef();
    for (int i=0; i<300; ++i) b3CreateBody(world, &spare);
    b3World_Step(world, 1.0f/60, 4);
    REQUIRE(b3Contact_IsValid(id), 68);
  }
  if (fat) {
    for (float x : {4.10f, 4.14f}) {
      b3Body_SetTransform(body, {x,1,0}, b3Quat_identity);
      if (batchedFat && x == 4.10f) {
        if (zeroFat) b3World_Step(world, 0.0f, 4);
        continue;
      }
      b3World_Step(world, 1.0f/60, 4);
      bool valid = b3Contact_IsValid(id);
      std::printf("fat_x=%g valid=%d\n", x, int(valid));
      REQUIRE(valid, 64);
      REQUIRE(b3Contact_GetData(id).manifoldCount == 0, 65);
    }
  }
  if (growFat) {
    auto spare = b3DefaultBodyDef();
    for (int i=0; i<300; ++i) b3CreateBody(world, &spare);
    b3World_Step(world, 1.0f/60, 4);
    REQUIRE(b3Contact_IsValid(id), 67);
  }
  if (materialFat) {
    b3Shape_SetFriction(shape, 0.4f);
    b3World_Step(world, 1.0f/60, 4);
    REQUIRE(b3Contact_IsValid(id), 66);
  }
  if (unread) {
    // No harvest through the gap and replacement of all physical child roots.
    for (int i = 0; i < 3; ++i) b3World_Step(world, 1.0f/60, 4);
    b3Body_SetTransform(body, {3,1,0}, b3Quat_identity);
    b3World_Step(world, 1.0f/60, 4);
    REQUIRE(b3Contact_IsValid(id), 52);
    REQUIRE(b3Body_GetContactData(body, data, 2) == 1 && !same(data[0].contactId, id), 53);
    REQUIRE(b3Contact_GetData(id).manifoldCount == 0, 61);
  }
  b3Body_SetTransform(body, {10,1,0}, b3Quat_identity);
  b3World_Step(world, 1.0f/60, 4);
  if (!unread) REQUIRE(!b3Contact_IsValid(id), 25);
  b3Body_SetTransform(body, {compound ? -3.0f : 0.0f,1,0}, b3Quat_identity);
  b3World_Step(world, 1.0f/60, 4);
  REQUIRE(b3Body_GetContactData(body, data, 2) == 1, 26);
  auto replacement = data[0].contactId;
  REQUIRE(b3Contact_IsValid(replacement) && !same(replacement, id) && !b3Contact_IsValid(id), 27);
  b3DestroyBody(body);
  REQUIRE(!b3Contact_IsValid(replacement), 28);
  b3DestroyWorld(world);
  REQUIRE(!b3Contact_IsValid(replacement), 29);
  if (compoundData) b3DestroyCompound(compoundData);
  std::puts("contact-lifetime=pass");
}
