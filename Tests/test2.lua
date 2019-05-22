-- This gives [0]=0, [1]=1, [2]=2, [3]="b", ["a"]=3
list = { -1, 2, a=3, "b", [0]=0 }

print(list[0])
print(list[1])
print(list[2])
print(list["a"])
print(list[3])

io.write("Size of list: ")
print(#list)

print "2 + (-(-1)) = "
print(list[2] + -list[1])
