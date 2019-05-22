list = { 6, 5, 2 }

repeat
	done = false
	count = #list-1
	for i=1, count do
		io.write("list[1]: ", list[1], ", list[2]: ", list[2], ", list[3]: ", list[3], "\n")
		io.write("i: ", i, ", list[i]: ", list[i], ", list[i+1]: ", list[i+1], "\n")
		if list[i] > list[i + 1] then
			--for j = 1,3 do
			--	io.write(list[j])
			--	io.write(", ")
			--end
			--io.write("\n")
			if i > 1 then
				tmp2 = list[i-1]
			end
			tmp = list[i]
			--io.write("  tmp: ")
			--print(tmp)
			list[i] = list[i+1]
			io.write("l=list 	list[1]: ", list[1], ", list[2]: ", list[2], ", list[3]: ", list[3], "\n")
			list[i+1] = tmp
			io.write("l=tmp  	list[1]: ", list[1], ", list[2]: ", list[2], ", list[3]: ", list[3], "\n")
			io.write("       	i=", i, "  list[i]: ", list[i], ", list[i+1]: ", list[i+1], "\n")
			done = true
		end
	end
until done == false

print "-----------------------------------------------------"
--for i = 1,#list do
--	print (list[i])
--end

