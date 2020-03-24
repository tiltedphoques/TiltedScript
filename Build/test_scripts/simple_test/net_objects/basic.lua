OBJECT.Properties = {
    TestProperty = { Replicates = true, Default = 42 }
}

OBJECT.NetRPCs = {
    TestRpc = {
        OnMaster = function (self, a)
            print(a)
            return true
        end,
        OnProxy = function(self, a)
            print("proxyyyy " .. tostring(a))
        end
    }
}

function OBJECT:OnActivate()
    print("active")
    self.NetRPCs.TestRpc(425)
end
