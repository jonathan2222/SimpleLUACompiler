list = { 6, 5, 2 }

i = 1
tmp = list[i]
list[i] = list[i+1]
list[i+1] = tmp
-- 5 6 2

i = 2
tmp = list[i]
list[i] = list[i+1]
list[i+1] = tmp

-- 5 2 6

print "-----------------------------------------------------"

for i = 1,#list do
	print (list[i])
end

-- 5 2 6

i = 1
print("<<<tmp", tmp)
tmp = list[1]
print(">>>tmp", tmp)
list[1] = list[2]
list[2] = tmp
print("tmp", tmp)

-- 2 5 6

print "-----------------------------------------------------"

for i = 1,#list do
	print (list[i])
end
--repeat
--	done = true
--	count = #list
--	if list[i]==2 then
--		done = false
--	end
--	i = i +1
--until done == false
