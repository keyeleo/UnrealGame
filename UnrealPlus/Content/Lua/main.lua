local Asset=import("AssetLibrary")

local actor={}

-- override event from blueprint
function actor:ReceiveBeginPlay()
    -- self.bCanEverTick = true
    self.bCanBeDamaged = false
    self.Super:ReceiveBeginPlay()

    print('-------- main run');

    local path="Meshes/Linear_Stair_StaticMesh.uasset";
    local obj=Asset.SyncLoadAsset(path, "");
    print("---------mesh=", obj);

    obj=self:GetWorld();
    print("---------world=", obj);

    path = "Blueprint'/Game/Blueprints/Stairs.Stairs_C'";
    obj=Asset.SpawnActor(self,path);
    print("---------actor=", obj);
end

return actor