
list = { 5, 4, 3, 1 }
count = #list

itemCount=count - 1
for i = 1, itemCount do
	print("Out:", i)
	--x = list[i+1]
	--if list[i] > list[i + 1] then
		print("In: ", i)
		list[i+1] = list[i]
		--list[i] = list[i+1]
		--list[i+1] = x
		--list[i], list[i + 1] = list[i + 1], list[i]
		--hasChanged = true
	--end
end