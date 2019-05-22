print "enter a number:"
n = io.read("*number")
for i = 2,n 
do
  if n%i==0 then
	io.write(i)
	io.write(" is a factor of ")
	print(n)
  elseif i%3==0 then
	io.write(i)
	io.write(" is divisiable by 3\n")
  else
	io.write(i)
	io.write " is not divisiable by 3 and is not a factor of "
	print(n)
  end
end