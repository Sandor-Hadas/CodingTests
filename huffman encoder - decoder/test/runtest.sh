echo "Running test of Huffman code example written for foodie.fm by Sandor Hadas"
echo ""

cd ..
make
cd test

echo ""
echo "Test 1 - simple ASCII characters"
../Huffman -e -i go.txt -o go.txt.encoded -c go.codetree
../Huffman -d -i go.txt.encoded -o go.txt.decoded -c go.codetree
echo "diff of original and decoded files (must be nothing):"
diff go.txt go.txt.decoded
echo "-- end of diff --"
echo ""

echo "Test 2 - all 256 characters (file has one character each in it)"
../Huffman -e -i nastytest.txt -o nastytest.txt.encoded -c nastytest.codetree
../Huffman -d -i nastytest.txt.encoded -o nastytest.txt.decoded -c nastytest.codetree
echo "diff of original and decoded files (must be nothing):"
diff nastytest.txt nastytest.txt.decoded
echo "-- end of diff --"
echo ""

echo "Test 3 - merging three files into one and decoding it"
../Huffman -e -t -i merge1.txt -i merge2.txt -i merge3.txt -o merged.txt.encoded -c merged.codetree
../Huffman -d -i merged.txt.encoded -o merged.txt -c merged.codetree
echo "diff expected output and output files (must be nothing):"
diff merged.txt merged_expected.txt
echo "-- end of diff --"
echo ""

echo "Test 4 - all 256 characters, large, complex file (.exe)"
../Huffman -e -i 1.exe -o 1.exe.encoded -c 1.codetree
../Huffman -d -i 1.exe.encoded -o 1.exe.decoded -c 1.codetree
echo "diff of original and decoded files (must be nothing):"
diff 1.exe 1.exe.decoded
echo "-- end of diff --"
echo ""

echo "Test 5 - all 256 characters, large, complex file (file is windows apache.dll)"
../Huffman -e -i apache.dll -o apache.dll.encoded -c apache.codetree
../Huffman -d -i apache.dll.encoded -o apache.dll.decoded -c apache.codetree
echo "diff of original and decoded files (must be nothing):"
diff apache.dll apache.dll.decoded
echo "-- end of diff --"
echo ""


echo "End test"
