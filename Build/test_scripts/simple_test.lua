print("lua hello")

f = basic.new()
f.Properties.TestProperty = 102
print(f.Properties.TestProperty)
f:OnActivate()