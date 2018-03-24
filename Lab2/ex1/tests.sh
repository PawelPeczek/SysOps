printf "\n-----\tCOPY TESTS\t-----\n" | tee -a test_results.txt
printf "Generating file...\n"
bin/app g s copy_test_origin 512 2000
printf "File genarated [OK]\n"
printf "\nCopy system calls, block size = 4B\n" | tee -a test_results.txt
bin/app c s copy_test_origin copy1 4 | tee -a test_results.txt
printf "\nCopy system calls, block size = 512B\n" | tee -a test_results.txt
bin/app c s copy_test_origin copy2 512 | tee -a test_results.txt
printf "\nCopy system calls, block size = 4096B\n" | tee -a test_results.txt
bin/app c s copy_test_origin copy3 4096 | tee -a test_results.txt
printf "\nCopy system calls, block size = 8192B\n" | tee -a test_results.txt
bin/app c s copy_test_origin copy4 8192 | tee -a test_results.txt
printf "\nCopy library calls, block size = 4B\n" | tee -a test_results.txt
bin/app c l copy_test_origin copy5 4 | tee -a test_results.txt
printf "\nCopy library calls, block size = 512B\n" | tee -a test_results.txt
bin/app c l copy_test_origin copy6 512 | tee -a test_results.txt
printf "\nCopy library calls, block size = 4096B\n" | tee -a test_results.txt
bin/app c l copy_test_origin copy7 4096 | tee -a test_results.txt
printf "\nCopy library calls, block size = 8192B\n" | tee -a test_results.txt
bin/app c l copy_test_origin copy8 8192 | tee -a test_results.txt
printf "\n-----\tCOPY TESTS END\t-----\n" | tee -a test_results.txt
rm -f copy* copy_test_origin
printf "\n-----\tSORT TESTS\t-----\n" | tee -a test_results.txt
printf "Generating files...\n"
bin/app g s sort_test_origin_4_1000_s 4 1000
bin/app g s sort_test_origin_4_2000_s 4 2000
bin/app g s sort_test_origin_512_1000_s 512 1000
bin/app g s sort_test_origin_512_2000_s 512 2000
bin/app g s sort_test_origin_4096_1000_s 4096 1000
bin/app g s sort_test_origin_4096_2000_s 4096 2000
bin/app g s sort_test_origin_8192_1000_s 8192 1000
bin/app g s sort_test_origin_8192_2000_s 8192 2000
cp sort_test_origin_4_1000_s sort_test_origin_4_1000_l
cp sort_test_origin_4_2000_s sort_test_origin_4_2000_l
cp sort_test_origin_512_1000_s sort_test_origin_512_1000_l
cp sort_test_origin_512_2000_s sort_test_origin_512_2000_l
cp sort_test_origin_4096_1000_s sort_test_origin_4096_1000_l
cp sort_test_origin_4096_2000_s sort_test_origin_4096_2000_l
cp sort_test_origin_8192_1000_s sort_test_origin_8192_1000_l
cp sort_test_origin_8192_2000_s sort_test_origin_8192_2000_l

printf "File genarated [OK]\n"
printf "\n\n--\tRecord size: 4B\t--\n" | tee -a test_results.txt
printf "\nSort system calls, block size = 4B, elems = 1000 \n" | tee -a test_results.txt
bin/app s s sort_test_origin_4_1000_s 4 | tee -a test_results.txt
printf "\nSort lib calls, block size = 4B, elems = 1000 \n" | tee -a test_results.txt
bin/app s l sort_test_origin_4_1000_l 4 | tee -a test_results.txt
printf "\nSort system calls, block size = 4B, elems = 2000 \n" | tee -a test_results.txt
bin/app s s sort_test_origin_4_2000_s 4 | tee -a test_results.txt
printf "\nSort lib calls, block size = 4B, elems = 2000 \n" | tee -a test_results.txt
bin/app s l sort_test_origin_4_2000_l 4 | tee -a test_results.txt

printf "\n\n--\tRecord size: 512B\t--\n" | tee -a test_results.txt
printf "\nSort system calls, block size = 512B, elems = 1000 \n" | tee -a test_results.txt
bin/app s s sort_test_origin_512_1000_s 512 | tee -a test_results.txt
printf "\nSort lib calls, block size = 512B, elems = 1000 \n" | tee -a test_results.txt
bin/app s l sort_test_origin_512_1000_l 512 | tee -a test_results.txt
printf "\nSort system calls, block size = 512B, elems = 2000 \n" | tee -a test_results.txt
bin/app s s sort_test_origin_512_2000_s 512 | tee -a test_results.txt
printf "\nSort lib calls, block size = 512B, elems = 2000 \n" | tee -a test_results.txt
bin/app s l sort_test_origin_512_2000_l 512 | tee -a test_results.txt

printf "\n\n--\tRecord size: 4096B\t--\n" | tee -a test_results.txt
printf "\nSort system calls, block size = 4096B, elems = 1000 \n" | tee -a test_results.txt
bin/app s s sort_test_origin_4096_1000_s 4096 | tee -a test_results.txt
printf "\nSort lib calls, block size = 4096B, elems = 1000 \n" | tee -a test_results.txt
bin/app s l sort_test_origin_4096_1000_l 4096 | tee -a test_results.txt
printf "\nSort system calls, block size = 4096B, elems = 2000 \n" | tee -a test_results.txt
bin/app s s sort_test_origin_4096_2000_s 4096 | tee -a test_results.txt
printf "\nSort lib calls, block size = 4096B, elems = 2000 \n" | tee -a test_results.txt
bin/app s l sort_test_origin_4096_2000_l 4096 | tee -a test_results.txt

printf "\n\n--\tRecord size: 8192B\t--\n" | tee -a test_results.txt
printf "\nSort system calls, block size = 8192B, elems = 1000 \n" | tee -a test_results.txt
bin/app s s sort_test_origin_8192_1000_s 8192 | tee -a test_results.txt
printf "\nSort lib calls, block size = 8192B, elems = 1000 \n" | tee -a test_results.txt
bin/app s l sort_test_origin_8192_1000_l 8192 | tee -a test_results.txt
printf "\nSort system calls, block size = 8192B, elems = 2000 \n" | tee -a test_results.txt
bin/app s s sort_test_origin_8192_2000_s 8192 | tee -a test_results.txt
printf "\nSort lib calls, block size = 8192B, elems = 2000 \n" | tee -a test_results.txt
bin/app s l sort_test_origin_8192_2000_l 8192 | tee -a test_results.txt

rm -f sort_test_origin_*