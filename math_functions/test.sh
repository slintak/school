#! /bin/sh

./proj2 -h
echo "----------\n"

echo "--- Chybny vstup\n"
echo "./proj2 --tanh 5 <<< \"bla\""
./proj2 --tanh 5 <<< "bla"
echo "./proj2 --tanh 5 <<< \"nan\""
./proj2 --tanh 5 <<< "nan"
echo "./proj2 --tanh 5 <<< \"0,4\""
./proj2 --tanh 5 <<< "0,4"


echo "./proj2 --logax 5 10 <<< \"bla\""
./proj2 --logax 5 10 <<< "bla"
echo "./proj2 --logax 5 10 <<< \"nan\""
./proj2 --logax 5 10 <<< "nan"
echo "./proj2 --logax 5 10 <<< \"0,4\""
./proj2 --logax 5 10 <<< "0,4"

echo "\n--- Mimo povoleny rozsah\n"
echo "./proj2 --logax 10 10 <<< \"-3.0\""
./proj2 --logax 10 10 <<< "-3.0"
echo "./proj2 --logax 10 -1 <<< \"1.0\""
./proj2 --logax 10 -1 <<< "1.0"
echo "./proj2 --logax 10 2 <<< \"0\""
./proj2 --logax 10 2 <<< "0"

echo "\n--- Spravny vstup\n"
echo "./proj2 --tanh 5 <<< \"0.5\""
./proj2 --tanh 5 <<< "0.5"
echo "./proj2 --tanh 5 <<< \"-100\""
./proj2 --tanh 5 <<< "-100"
echo "./proj2 --logax 5 2 <<< \"16.0\""
./proj2 --tanh 5 <<< "0.5"
echo "./proj2 --wam <<< \"1.5 1.0 2.5 2.0 3.5 3.0\""
./proj2 --wam <<< "1.5 1.0 2.5 2.0 3.5 3.0"
echo "./proj2 --wqm <<< \"1.5 1.0 2.5 2.0 3.5 3.0\""
./proj2 --wqm <<< "1.5 1.0 2.5 2.0 3.5 3.0"
