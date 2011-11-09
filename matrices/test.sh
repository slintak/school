#! /bin/bash

#export VG="valgrind --tool=memcheck -q --leak-check=yes --show-reachable=yes"
export VG="valgrind --tool=memcheck -q --leak-check=yes"

echo "## Kompiluji"
make

echo "## Testuji --test"
./proj3 --test txt/vektor.txt | diff -Bw - txt/vektor.txt
./proj3 --test txt/matrices.txt | diff -Bw - txt/matrices.txt

echo "## Testuji --vadd"
diff -Bw <(./proj3 --vadd txt/a.txt txt/b.txt) <(echo -e "1\n8\n9 -5 9 9 9 9 9 9")

echo "## Testuji --vscal"
diff -Bw <(./proj3 --vscal txt/a.txt txt/b.txt) <(echo "92")

echo "## Testuji --mmult"
diff -Bw <(./proj3 --mmult txt/mul_a.txt txt/mul_b.txt) <(echo -e "2\n3 3\n19 7 20\n33 12  39\n33 13 12")

echo "## Testuji --bubbles"
diff -Bw <(./proj3 --bubbles txt/bubbles.txt) <(echo "5")
diff -Bw <(./proj3 --bubbles txt/bubbles2.txt) <(echo "6")
diff -Bw <(./proj3 --bubbles txt/vektor.txt) <(echo "2")

echo "## Testuji --extbubbles"
diff -Bw <(./proj3 --extbubbles txt/bubbles_3D.txt) <(echo "3")

echo "## Testuji na prazdnem souboru"
$VG ./proj3 --test txt/empty.txt

echo "## Testuji na neexistujici soubor"
$VG ./proj3 --test blabla.txt

echo "## Testuji soubor s cybnymi daty"
./proj3 --test txt/matrices_err.txt

echo "## Testuji Valgrindem"
echo "# Matice A:"
cat txt/mul_a.txt
echo "# Matice B:" 
cat txt/mul_b.txt
echo "# Soustim Valgrind"
$VG ./proj3 --mmult txt/mul_a.txt txt/mul_b.txt

echo "## Testuji s ulimit"
(ulimit -v 75000; $VG ./proj3 --mmult txt/big.txt txt/matrix.txt)
