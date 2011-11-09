/******************************************************************************
 *
 * Projekt cislo 1 do predmetu BIZP -- Jednoducha komprese textu
 *
 * == Zadani ulohy ==
 * Vytvorte program ktery bude dale popsanym postupem komprimovat nebo
 * dekomprimovat text podle parametry zadanych z prikazoveho radku pri
 * spousteni. Jednotlive pozadovane parametry jsou popsany dale.
 *
 * Priklad komprese pro N = 3:
 * 	"Uiiiiiii, blablabla" a "blebleble" a "hophop hophop"
 * 	"U2iiii, 3bla" a "3ble" a "2hop 2hop"
 *
 * Pouziti:
 * 	./proj1 -c N	- Vyhledava opakujici se bloky o delce N a komprimuje
 * 	./proj1 -d N	- Stejne jako -c, ale dekomprimuje
 * 	./proj1 -h	- Vytiskne napovedu
 * 	./proj1 --extra	- Zatim neexistuje
 *
 * Autor: 	Vlastimil Slintak <xslint01@stud.feec.vutbr.cz>
 * AutorID:	121035@FEKT
 * Datum: 	Tue, 05 Oct 2010 14:05:37 +0200
 * Verze: 	1.0
 ******************************************************************************/

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

/**
 * Akce programu
 */
enum actions {
	A_CODE,		// Komprese textu
	A_DECODE,	// Dekomprese textu
	A_HELP,		// Tisk napovedy
//	A_EXTRA,	// Nejaka funkce navic
	A_ERR		// Chybove hlaseni
};

/**
 * Chybove kody, ktere mohou vracet funkce
 */
enum errors {
	E_OK,		// Vsechno probehlo OK
	E_CL,		// Chyba parametru prikazoveho radku
	E_UNKW,		// Neznama chyba
	E_STR,		// Chyba vstupniho retezce
	E_INT,		// Uzivatel zadal spatne cislo. Nelze prevest na uint
	E_MEM,		// Chyba pameti
	E_EOF,		// Konec vstupniho retezce
};

/**
 * Ke kazdemu chybovemu kodu je prirazen retezec popisujici chybu.
 */
const char *ERROR_MSG[] = {
	"Vse probehlo bez chyby.\n",
	"Spatne zadane vstupni parametry!\n",
	"Nastala neznama chyba!\n",
	"Chyba vstupniho retezce. Nelze zpracovat.\n",
	"Spatne zadane vstupni parametry. Nelze prevest cislo na uint!\n",
	"Nemohu alokovat pamet. Pravdepodobne neni dostatek pameti?\n"
};

/**
 * Napoveda k programu
 */
int HELP_MSG_SIZE = 10;
const char *HELP_MSG[] = {
	"Program je soucasti projektu cislo 1 do predmetu BIZP:",
	"\tJednoducha komprese textu\n",
	"\n",
	"Autor: Vlastimil Slintak, 121035@FEKT\n",
	"\n",
	"Parametry programu jsou:\n",
	" -h\t\tVytiskne tuto napovedu a skonci\n",
	" -c N\t\tNacita znaky z stdin, vyhledava bloky o N znacich a komprimuje\n",
	" -d N\t\tNacita znaky z stdin a dekomprimuje\n",
	" --extra\tNedela nic uzitecneho.\n"
};

/**
 * Do teto struktury se ukladaji parametry a akce prikazove radky
 */
typedef struct parameters {
	unsigned int N;	// Delka bloku vstupniho textu
	int action;	// Akce, ktera se ma vykonat
	int error;	// Chybovy kod programu (pouze pokud nastala chyba)
} t_parameters;

/**
 * Struktura posuvneho okenka, ktere se pouziva pro kompresi textu.
 * 'data' je pole o velikost 'size'. Do techto dat se pristupuje jako
 * do kruhoveho registru pomoci makra W_POS(w, i)
 */
typedef struct _window {
	int *data;
	unsigned int size,
		     position;
} t_window;

#define W_EMPTY	-1
#define W_POS(w, i)	(((w)->position+(i))%(w)->size)

/**
 * Funkce atoi() z stdlib.h vraci pouze int a nedokaze rict,
 * pokud nastala pri prevodu chyba. Proto radeji vyuzijeme
 * tuto vlastni funkci
 *
 * V pripade neuspechu pri prevodu retezce na cislo vracime
 * false. Pokud je vse v poradku, pak true a vysledne cislo
 * je predano pres ukazatel 'x'.
 *
 * @param s Ukazatel na retezec znaku, ktere chceme prevest.
 * @param x ukazatel na uint. Zde se ulozi cislo.
 */
bool safe_str_to_uint(const char *s, unsigned int *x) {
	// Chceme cislo v desitkove soustave.
	const unsigned int base = 10;

	// Projdeme retezec 's' od zacatku do konce a
	// zpracovavame po znacich.
	*x = 0;
	for ( ; *s != '\0'; s++) {
		unsigned char c = *s;

		// Pokud se v retezci vyskytuje neco jineho
		// nez cislo, vracime chybu.
		if (!isdigit(c)) return false;

		// Jednoduchy prevod znaku na cislo.
		unsigned char digit = c - '0';

		// Nesmi dojit k preteceni uint.
		if (*x > UINT_MAX / base)
			return false;
	
		// Posuneme cislo o jeden rad doleva.
		*x *= 10;

		if (*x > UINT_MAX - digit)
			return false;

		*x += digit;
	}
	return true;
}

/**
 * Funkce zpracuje vstupni parametry prikazoveho radku a vrati akci, kterou
 * uzivatel vyzadal.
 * Velmi jednoducha implementace.
 */
t_parameters get_params(int argc, char *argv[]) {
	t_parameters params = {0, A_ERR, E_CL};

	//
	// -h --> napoveda
	if(argc == 2 && strcmp("-h", argv[1]) == 0) {
		params.action = A_HELP;
		params.error = E_OK;
	} else if(argc == 3) {
	//
	// -c --> komprese textu
		if(strcmp("-c", argv[1]) == 0) {
			params.action = A_CODE;
			params.error = E_OK;
			if(!safe_str_to_uint(argv[2], &(params.N)) || params.N == 0) {
				params.action = A_ERR;
				params.error = E_INT;
			}
	//
	// -d --> dekomprese textu
		} else if(strcmp("-d", argv[1]) == 0) {
			params.action = A_DECODE;
			params.error = E_OK;
			if(!safe_str_to_uint(argv[2], &(params.N)) || params.N == 0) {
				params.action = A_ERR;
				params.error = E_INT;
			}
		}
	}
	return params;
}

/**
 * Alokuje pamet pro strukturu t_window, vyplni pocatecni data a vrati
 * ukazatel na nove vzniklou strukturu.
 * w->data je pole integeru, kterou budeme dale nazyvat 'kruhovy registr'.
 *
 * @param size Velikost okenka
 */
t_window *create_window(unsigned int size) {
	// Alokujeme potrebnou pamet pro strukturu a zaroven
	// overime, jestli jsme ji dostali.
	t_window *w = malloc(sizeof *w);
	if(w == NULL)
		return NULL;

	// Pamet pro data. V nasem pripade budeme nacitat
	// pouze znaky, takze by stacilo 'char', ale potrebujeme
	// ukladat i informace o konci a o prazdne bunce.
	w->data = malloc(size * (sizeof *w->data));
	if(w->data == NULL) {
		free(w);
		return NULL;
	}

	// Vymazeme nove alokovanou pamet.
	for(unsigned int i = 0; i < size; i++)
		w->data[i] = W_EMPTY;

	// Ulozime si velikost kruhoveho registru a pozici, na ktere
	// se nachazeji posledni ulozena data.
	w->size = size;
	w->position = 0;
	return w;
}

/**
 * Uvolni alokovanou pamet okenka 'w'.
 *
 * @param w Ukazatel na strukturu t_window
 */
void delete_window(t_window *w) {
	if(w != NULL) {
		free(w->data);
		free(w);
	}
}

/**
 * Nacte do okenka 'w' dalsich 'number_of_chars' znaku. Okenko se chova jako
 * kruhovy registr, takze stare znaky jdou premazany.
 *
 * @param w Ukazatel na strukturu t_window
 */
int move_window(t_window *w, unsigned int number_of_chars) {
	if(w == NULL)
		return -1;

	unsigned int position = w->position;
	// Zacneme nacitat prislusny pocet znaku...
	for(unsigned int i = 0; i < number_of_chars; i++) {
		// Vypocitame na kterou pozici v kruhovem
		// registru ulozime nacteny znak.
		position = W_POS(w, i+1);
		
		// Pokud jsme uz drive narazili na konec vstupniho souboru,
		// nebudeme nacitat dale.
		if(w->data[position] == E_EOF)
			return E_EOF;

		// Nacteme jeden znak z stdin, overime jestli se jedna o
		// platny vstupni znak (neni to cislo) a take jestli jsme
		// nenacetli EOF.
		int c;
		if((c = getchar()) != EOF) {
			// Na vstupu nesmi byt cisla!
			if(isdigit(c)) return E_STR;
			// Nacteny znak ulozime do okenka
			w->data[position] = c;
		} else
			w->data[position] = E_EOF;
	}

	// Ulozime posledni pozici v kruhovem registru.
	w->position = position;
	return E_OK;
}

/**
 * Vraci mensi ze dvou cisel.
 * Mohlo by byt i jako makro.
 */
unsigned int uint_min(unsigned int a, unsigned int b) {
	return a < b ? a : b;
}

/**
 * Vytiskne 'number_of_chars' znaku z okenka 'w' na stdout. Navic, pokud je
 * promenna 'number_of_blocks' vetsi jak 1, vytiskne nejdrive ji.
 * Napriklad:
 * 	Nase 'okenko' ma velikost 6 znaku a obsahuje 'abcdef'.
 *
 * 	print_window(w, 3, 1);
 * 	stdout:	abc
 *
 * 	print_window(w, 2, 5);
 * 	stdout: 5ab
 *
 * 	print_window(w, 10, 3);
 * 	stdout: 3abcdefabcd
 *
 * @param w Ukazatel na strukturu t_window
 * @param number_of_chars Pocet znaku, ktere se maji vytisknout z okenka
 * @param number_of_blocks Cislo, ktere se vytiskne na stdout
 */
void print_window(
		t_window *w,
		unsigned int number_of_chars,
		unsigned int number_of_blocks) {

	// Pokud jsme dostali neexistujici okenko, koncime.
	if(w == NULL) return;

	// Dokud mame co tisknou...
	while (number_of_blocks > 0) {
		// Pokud je 'number_of_blocks' vetsi jak 9, vytiskneme
		// pouze 9 a zbytek nechame na dalsi iteraci
		unsigned int to_print = uint_min(number_of_blocks, 9);

		// Pokud mame stale vice jak 1 opakovani bloku, vytiskneme
		// nejdrive cislo...
		if (to_print > 1)
			putchar(to_print + '0');
		// ...a potom znaky
		for(unsigned int i = 0; i < number_of_chars; i++)
			putchar(w->data[W_POS(w, i+1)]);

		// Pocet znaku, ktere jeste zbyva vytisknout.
		number_of_blocks -= to_print;
	}
}

/**
 * Funkce testuje okenko 'w'. 'Okenko' je obycejne pole znaku, ktere pouzivame
 * jako kruhovy registr. Postupne do nej nacitame znaky a testujeme vyskyt
 * opakujicich se bloku. Napriklad:
 * 	Mame okenko o velikosti 6 znaku a jeho obsah je nasledujici...
 *
 * 	+---+---+---+---+---+---+
 * 	| a | b | c | a | b | c |
 * 	+---+---+---+---+---+---+
 * 	  0   1   2   3   4   5
 *
 * 	Blok 'abc' se opakuje, vratime tedy true.
 *
 * 	+---+---+---+---+---+---+
 * 	| a | b | c | a | a | a |
 * 	+---+---+---+---+---+---+
 * 	  0   1   2   3   4   5
 *
 * 	Blok 'abc' se neopakuje, vracime false.
 *
 * @params w Ukazatel na strukturu t_window
 */
bool test_window(t_window *w) {
	if(w == NULL) return false;

	// Testujeme stejne znaky v prvni a druhe polovine okna.
	unsigned int half = w->size/2;
	for(unsigned int i = 0; i < half; i++)
		if(w->data[W_POS(w, i)] != w->data[W_POS(w, i+half)])
			return false;
	return true;
}

/**
 * Funkce nacita text z stdin a snazi se jej zakodovat. Vystup se tiskne na
 * stdout. Vstupni text mohou byt pouze znaky ASCII krome cisel. Vystupni text
 * pak obsahuje ASCII znaky i s cisly.
 *
 * Priklad pro N=3. Okenko bude mit velikost 6.
 * stdin: "Uiiiiiii, blabla."
 *         ---+++		Blok se neopakuje, vytiskni 1 znak a posun se.
 *          ---+++		Opakovani bloku. Posuneme se o 3 doprava.
 *             ---+++		Tady uz se neopakuje, vytiskneme cislo a 3znaky
 *                ---+++	Zadne opakovani, vytiskneme 1 znak o posun.
 *                 ---+++	...
 *                  ---+++	...
 *                   ---+++	Opakovani!
 *                      ---+++	Konec opakovani, vytiskneme cislo a 3znaky.
 *                         -X	Konec vstupu. Vytiskneme posledni znak.
 * stdout: U2iiii, 2bla.
 *
 * @params N Velikost jednoho bloku
 */
int code(unsigned int N) {
	// Vytvorime si 'posuvne okenko' o velikosti 2*N.
	t_window *w = create_window(2*N);
	if(w == NULL)
		return E_MEM;

	// Pocet bloku o velikosti N, ktere se opakuji ve vstupnim proudu.
	unsigned int number_of_blocks = 1;
	// Pocet znaku, o ktere posuneme 'okenko'.
	unsigned int move = 1;
	// Navratova hodnota funkce move_window()
	enum errors mw_return;

	// Na zacatku toto okenko vyplnime znaky z stdin.
	move_window(w, 2*N);

	// Nacitame vstupni proud, dokud nenarazime na konec souboru.
	do {
		// Pokud jsme narazili na dalsi opakujici se blok...
		if(test_window(w)) {
			// ...zapocitame ho...
			number_of_blocks++;
			// ...a posuneme okenko o N znaku doprava.
			move = N;
		} else {
			// V opacnem pripade vytiskneme blok znaku, pripadne
			// jeden znak, na stdout...
			move = (number_of_blocks > 1) ? N : 1;
			print_window(w, move, number_of_blocks);
			// A vynulujeme citac bloku.
			number_of_blocks = 1;
		}

		// Posuneme okenko...
		mw_return = move_window(w, move);
		// ...pokud jsme behem posunu narazili na neplatny
		// znak, vracime uzivateli chybu.
		if(mw_return == E_STR) return E_STR;
	// Koncime jakmile narazime na konec souboru.
	} while( mw_return != E_EOF );

	// Uvolnime pamet pouzitou na okenko.
	delete_window(w);
	return E_OK;
}

/**
 * Funkce bere text ze standardniho vstupu a snazi se jej dekodovat. Vstupni
 * text muze obsahovat pouze znaky ASCII vcetne cisel. Cislo ve vstupnim proudu
 * znaci pocet opakovani bloku.
 *
 * Napriklad: N = 3
 * 	stdin:	"3bla 2blek."
 * 	stdout:	"blablabla bleblek."
 *
 * Maximalni pocet opakovani bloku je 9, tedy ve vstupnim proudu se mohou
 * objevit pouze jednociferna cisla.
 *
 * Funkce vraci E_OK.
 *
 * @params N Velikost jednoho bloku
 */
int decode(unsigned int N) {
	int c;
	int *buff = malloc(N * sizeof *buff); // Buffer do ktereho se nacitaji bloky.
	if(buff == NULL)
		return E_MEM;

	// Budeme nacitat z stdin, dokud nenarazime na konec souboru.
	while((c = getchar()) != EOF) {
		// Pokud se nejedna o cislo...
		if(!isdigit(c))
			putchar(c); // ... vytiskneme jej na stdout.
		// Pokud jsme nacetli cislo...
		else {
			// Jednoduchy zpusob, jak udelat ze znaku cislo.
			unsigned int num_of_blocks = c - '0';
			
			// Do pripraveneho bufferu si nacteme potrebny
			// pocet znaku z stdin, musi to byt znaky ASCII,
			// krome cisel.
			for(unsigned int i = 0; i < N; i++) {
				buff[i] = getchar();
				if(buff[i] == EOF || isdigit(buff[i])) {
					free(buff);
					return E_STR;
				}
			}

			// Znaky z bufferu vytiskneme na stdout
			for(unsigned int i = 0; i < num_of_blocks; i++)
				for(unsigned int j = 0; j < N; j++)
					putchar(buff[j]);
		}
	}

	free(buff);
	return E_OK;
}

int main(int argc, char *argv[]) {
	t_parameters params = get_params(argc, argv);

	switch(params.action) {
		case A_ERR:
			fprintf(stderr, "%s", ERROR_MSG[params.error]);
			return 1;
		case A_HELP:
			for(int i = 0; i < HELP_MSG_SIZE; i++)
				fprintf(stdout, "%s", HELP_MSG[i]);
			break;
		case A_DECODE:
			if(decode(params.N) != E_OK) {
				fprintf(stderr, "%s", ERROR_MSG[E_STR]);
				return 1;
			}
			break;
		case A_CODE:
			if(code(params.N) != E_OK) {
				fprintf(stderr, "%s", ERROR_MSG[E_STR]);
				return 1;
			}
			break;
		default:
			fprintf(stderr, "%s", ERROR_MSG[E_UNKW]);
			return 1;

	}

	return 0;
}
