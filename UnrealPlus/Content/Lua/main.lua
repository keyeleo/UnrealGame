-- local Asset=import("AssetLibrary")

local actor={}

-- override event from blueprint
function actor:ReceiveBeginPlay()
    -- self.bCanEverTick = true
    self.bCanBeDamaged = false
    self.Super:ReceiveBeginPlay()

    print('-------- main run');

    -- local path="Meshes/Linear_Stair_StaticMesh.uasset";
    -- local obj=Asset.SyncLoadAsset(path, "");
    -- print("---------mesh=", obj);

    -- obj=self:GetWorld();
    -- print("---------world=", obj);

    -- path = "Blueprint'/Game/Blueprints/Stairs.Stairs_C'";
    -- obj=Asset.SpawnActor(self,nil,path);
    -- x,y,z=Asset.SpawnActor(self,nil,path);
    -- obj=Asset.SpawnActor(self,obj,"");
    -- print("---------actor=", obj);

    local world = self:GetWorld()
    -- local bpClass = slua.loadClass("/Game/TestActor.TestActor")
    local bpClass = slua.loadClass("/Game/Blueprints/Stairs.Stairs")

    local p = FVector(0,0,0)
    local aa = world:SpawnActor(bpClass,p,nil,nil)
    -- aa.Name = 'ActorCreateFromLua_'

    -- local actorClass = import("MyLuaActor")
    -- local p = FVector(math.random(-100,100),math.random(-100,100),0)
    -- local aa = world:SpawnActor(actorClass,p,nil,nil)

    print("---------actor=", aa);
end

return actor