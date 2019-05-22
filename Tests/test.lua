x = 2
for i=0,5,2 do
	io.write("i: ", i, ", x: ", i*x, "\n")
end

io.write "10 or 20: "
if 10 or 20 then
	io.write "true"
else
	io.write "false"
end
io.write "\n"

io.write "10 or error(): "
if 10 or error() then
	io.write "true"
else
	io.write "false"
end
io.write "\n"

io.write "nil or 'a': "
if nil or "a" then
	io.write "true"
else
	io.write "false"
end
io.write "\n"

io.write "nil and 10: "
if nil and 10 then
	io.write "true"
else
	io.write "false"
end
io.write "\n"

io.write "false and error(): "
if false and error() then
	io.write "true"
else
	io.write "false"
end
io.write "\n"

io.write "false and nil: "
if false and nil then
	io.write "true"
else
	io.write "false"
end
io.write "\n"

io.write "false or nil: "
if false or nil then
	io.write "true"
else
	io.write "false"
end
io.write "\n"

io.write "10 and 20: "
if 10 and 20 then
	io.write "true"
else
	io.write "false"
end
io.write "\n"