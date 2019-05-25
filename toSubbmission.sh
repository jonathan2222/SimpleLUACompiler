cp -r ./AST/ ../submission/ass2-comp/
cp binary.ll binary.yy main.cc Makefile ../submission/ass2-comp/
rm -r ../submission.tgz
cd ../submission/
tar -cvzf ../submission.tgz *
