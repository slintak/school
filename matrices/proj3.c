/******************************************************************************
 *
 * Projekt cislo 3 do predmetu BIZP -- Maticove operace
 *
 * Autor: 	Vlastimil Slintak <xslint01@stud.feec.vutbr.cz>
 * AutorID:	121035@FEKT
 * Datum: 	Thu, 30 Dec 2010 23:58:14 +0200
 * Verze: 	1.0
 ******************************************************************************/
#include "proj3.h"

bool load_number_from_file(FILE *fr, int *number) {
	if(fr == NULL)
		return false;

	// Nacteme cislo ze souboru, v pripade
	// neuspechu vracime false, v opacnem pripade
	// je cislo ulozeno do *number
	if(fscanf(fr, "%d", number) != 1)
		return false;

	return true;
}

bool load_header_from_file(FILE *fr, header_s *hdr) {
	if(fr == NULL || hdr == NULL)
		return false;

	// Prvni cislo v souboru je typ - matice, vektor
	// nebo vektor matic.
	if(!load_number_from_file(fr, &(hdr->type)))
		return false;

	// Predvyplnime rozmery matice na 1 1 1 1.
	for(int i = 0; i < MATRIX_HEADER; i++)
		hdr->dims[i] = 1;

	// Podle predem nacteneho typu nacteme i prislusny
	// pocet cislic z druheho radku souboru, ktere
	// predstavuji rozmery matice.
	switch(hdr->type) {
		case MATRICES:
			if(!load_number_from_file(fr, &(hdr->dims[MATRIX_Z])))
				return false;
		case MATRIX:
			if(!load_number_from_file(fr, &(hdr->dims[MATRIX_Y])))
				return false;
		case VECTOR:
			if(!load_number_from_file(fr, &(hdr->dims[MATRIX_X])))
				return false;
			break;
		// Pokud nebyl typ validni, vracime chybu.
		default:
			return false;
	}

	return true;
}

int matrix_get_dimension(header_s *header) {
	// Vrati maximalni rozmery matice
	switch(header->type) {
		case VECTOR:
			return header->dims[MATRIX_X];
		case MATRIX:
			return header->dims[MATRIX_Y] *
			       header->dims[MATRIX_X];
		case MATRICES:
			return header->dims[MATRIX_Y] *
			       header->dims[MATRIX_X] *
			       header->dims[MATRIX_Z];
	}
	return -1;
}

header_s matrix_make_header(int typ, int x, int y, int z) {
	// Ze zadanyc hodnot vytvori hlavicku matice.
	return (header_s){
		typ,
		{
			// C99 podle vseho tuto zvlastni syntax
			// povoluje.
			[MATRIX_X] = x,
			[MATRIX_Y] = y,
			[MATRIX_Z] = z
		}
	};
}

matrix_s *matrix_alloc_mem(header_s *header) {
	// Pamet pro samotnou matici
	matrix_s *M = malloc(sizeof *M);
	if(M == NULL)
		return NULL;

	// Pamet pro hlavicku
	M->header = malloc(sizeof *header);
	if(M->header == NULL) {
		free(M);
		return NULL;
	}
	// Zkopirujeme data z 'header' do nove vytvorene
	// matice M->header.
	memcpy(M->header, header, sizeof *header);

	// Nakonec alokujeme pamet pro potrebny pocet
	// dat.
	int dimension = matrix_get_dimension(header);
	M->data = malloc(sizeof(int) * dimension);
	if(M->data == NULL) {
		free(M->header);
		free(M);
		return NULL;
	}

	// Vratime vynulovanou matici.
	for(int i = 0; i < dimension; i++)
		M->data[i] = 0;

	return M;
}

void matrix_free_mem(matrix_s *M) {
	// Uvolni pamet matice.
	if(M != NULL) {
		if(M->data != NULL)
			free(M->data);
		if(M->header != NULL)
			free(M->header);
		free(M);
	}
}

bool matrix_save_number(matrix_s *M, int position, int number) {
	if(M == NULL)
		return false;

	// Nejprve skontrolujeme jestli neni zadana
	// pozice mimo rozmery matice.
	if(position < 0 && position >= matrix_get_dimension(M->header))
		return false;

	M->data[position] = number;
	return true;
}

/**
 * Funkce vrati cislo prvku na pozici 'position'
 * z matice 'M'.
 *
 * POZOR! V pripade jakekoliv chyby funkce vraci nulu!
 */
int matrix_get_number(matrix_s *M, int position) {
	if(M == NULL)
		return 0;

	// Kontrola mezi matice.
	if(position < 0 && position >= matrix_get_dimension(M->header))
		return 0;

	return M->data[position];
}

void matrix_print(matrix_s *M) {
	int dimension = matrix_get_dimension(M->header);
	int type = M->header->type;
	int x = M->header->dims[MATRIX_X];
	int y = M->header->dims[MATRIX_Y];
	int z = M->header->dims[MATRIX_Z];

	// Vytiskneme typ matice.
	printf("%d\n", type);
	// Podle typu vytiskneme i
	// jeji rozmery
	switch(type) {
		case MATRICES:
			printf("%d ", z);
		case MATRIX:
			printf("%d ", y);
		case VECTOR:
			printf("%d\n", x);
	}

	// A nakonec i samotna data.
	// Na konci kazdeho radku matice tiskneme
	// znak \n a na konci kazde matice tiskneme
	// znak \n (dulezite pro vektory matic)
	for(int i = 1; i <= dimension; i++) {
		printf("%d  ", matrix_get_number(M, i-1));
		if((i % x) == 0)
			printf("\n"); // Radky matice
		if((i % (x * y)) == 0)
			printf("\n"); // Radky vektoru matic
	}
}

matrix_s *load_data_from_file(char *filename) {
	// Pokusime se otevrit dany soubor
	FILE *fr = fopen(filename, "r");
	if(fr == NULL)
		return NULL;

	// Ukazatel na strukturu do ktere se budou ukladat data
	matrix_s *M = NULL;
	// Hlavicka matice se sklada z typu (vektor, matice, vektor matic)
	// a z jejich rozmeru.
	header_s *header = malloc(sizeof *header);
	// Pokusime se ze souboru nacist hlavicku
	if(load_header_from_file(fr, header)) {
		// Alokujeme pamet pro data matice.
		// Hlavicku jiz name, takze zname i rozmery matice.
		M = matrix_alloc_mem(header);
		if(M != NULL) {
			// Vypocitame pocet prvku cele matice
			int dimension = matrix_get_dimension(header);
			int number, i;
			// Zacneme nacitat jednotlive cisla ze souboru...
			for(i = 0; i < dimension; i++) {
				if(!load_number_from_file(fr, &number))
					break;
				// ... a ukladat je do pripravene struktury
				// na pozici i (pocitano od zacatku po radcich)
				matrix_save_number(M, i, number);
			}

			// Pokud jsme nacetli mene hodnot nez je velikost matice,
			// uvolnime pamet a vratime NULL -> Soubor je chybny.
			if(i < dimension) {
				matrix_free_mem(M);
				M = NULL;
			}
		}
	}
	// Uvolnime pamet pouzitou pro hlavicku.
	free(header);

	// Uzavreme soubor se kterym jsme pracovali.
	// Pokud se nepodari soubor uzavrit, pouze na
	// to upozornime uzivatele.
	if(fclose(fr) != 0)
		fprintf(stderr, "Nepodarilo se uzavrit soubor s daty.\n");

	return M;
}

int vector_scalar_mul(matrix_s *A, matrix_s *B, int *result) {
	// Zkontrolujeme jestli mame oba vektory
	if(A == NULL || B == NULL)
		return E_UNKW;

	// Jedna se o vektory?
	if(!IS_VECTOR(A) || !IS_VECTOR(B))
		return E_MATH;

	// Rozmer vektoru musi souhlasit
	int dimensions_a = matrix_get_dimension(A->header);
	int dimensions_b = matrix_get_dimension(B->header);
	if(dimensions_a != dimensions_b)
		return E_MATH;

	int num = 0;
	for(int i = 0; i < dimensions_a; i++)
		num += matrix_get_number(A, i) * matrix_get_number(B, i);

	*result = num;

	return E_OK;
}

int vector_addition(matrix_s *A, matrix_s *B, matrix_s *C) {
	// Zkontrolujeme jestli mame oba vektory
	if(A == NULL || B == NULL || C == NULL)
		return E_UNKW;

	// Jedna se o vektory?
	if(!IS_VECTOR(A) || !IS_VECTOR(B) || !IS_VECTOR(C))
		return E_MATH;

	// Rozmer vektoru musi souhlasit
	int dimensions_a = matrix_get_dimension(A->header);
	int dimensions_b = matrix_get_dimension(B->header);
	int dimensions_c = matrix_get_dimension(C->header);
	if((dimensions_a != dimensions_b) ||
	   (dimensions_a != dimensions_c))
		return E_MATH;

	int num;
	for(int i = 0; i < dimensions_a; i++) {
		num = matrix_get_number(A, i) + matrix_get_number(B, i);
		matrix_save_number(C, i, num);
	}

	return E_OK;
}

int matrix_multiplication(matrix_s *A, matrix_s *B, matrix_s **C) {
	// Zkontrolujeme jestli mame vsechny matice
	if(A == NULL || B == NULL)
		return E_UNKW;

	// Jedna se o matice?
	if(!IS_MATRIX(A) || !IS_MATRIX(B))
	return E_MATH;

	// Zkontrolujeme rozmery matic
	if(MATRIX_GET_X(A) != MATRIX_GET_Y(B))
		return E_MATH;

	// Neprve vytvorime novou matici vhodnych rozmeru.
	header_s hdr = matrix_make_header(
				MATRIX,
				A->header->dims[MATRIX_Y],
				B->header->dims[MATRIX_X],
				0);
	// Alokujeme pro ni pamet
	*C = matrix_alloc_mem(&hdr);
	if(*C == NULL)
		return E_MEM;

	int num;
	for(int i = 0; i < MATRIX_GET_X(*C); i++) {
		for(int j = 0; j < MATRIX_GET_Y(*C); j++) {
			num = 0;
			for(int k = 0; k < MATRIX_GET_X(A); k++) {
			  num += matrix_get_number(A, MATRIX_COORDINATES(A, k, i, 0)) *
				 matrix_get_number(B, MATRIX_COORDINATES(B, j, k, 0));
			}
			matrix_save_number(*C, MATRIX_COORDINATES(*C, j, i, 0), num);
		}
	}

	return E_OK;
}

bool matrix_bubbles(matrix_s *M, matrix_s *visited, int position) {
	// Zkontrolujeme hranice matice
	// Pokud jsme mimo, vracime false
	if(position < 0 || position >= matrix_get_dimension(M->header))
		return false;
	// Pokud jsme jiz tuto bunku kontrolovali, koncime
	if(matrix_get_number(visited, position) == 1)
		return false;

	// Oznacime prave kontrolovanou bunku za jiz
	// navstivenou.
	matrix_save_number(visited, position, 1);

	// Pokud jsme v bubline zkontrolujeme i
	// ctyr/sesti okoli
	if(matrix_get_number(M, position) == 0) {
		// Pokud jsme na pravem okraji, nekontrolujeme
		// bunku vpravo
		if(!(((position+1) % MATRIX_GET_X(M)) == 0))
			matrix_bubbles(M, visited, position+1);
		// pokud jsme na levem okraji, nekontrolujeme
		// bunku vlevo
		if(!((position % MATRIX_GET_X(M)) == 0))
			matrix_bubbles(M, visited, position-1);
		// Bunka nahore.
		matrix_bubbles(M, visited, position-MATRIX_GET_X(M));
		// Bunka dole.
		matrix_bubbles(M, visited, position+MATRIX_GET_X(M));

		// Pokud nemame vektor matic, koncime,
		// jinak pokracujeme jeste dalsimy smery
		if(!IS_MATRICES(M))
			return true;

		// Bunka vepredu.
		matrix_bubbles(M,
				visited,
				position+MATRIX_GET_X(M)*MATRIX_GET_Y(M));
		// Bunka vzadu.
		matrix_bubbles(M,
				visited,
				position-MATRIX_GET_X(M)*MATRIX_GET_Y(M));
		return true;
	}
	return false;
}

int matrix_count_bubbles(matrix_s *M, unsigned int *result) {
	if(M == NULL)
		return E_UNKW;

	// Zde budeme oznacovat na kterem krvku matice jsme
	// jiz byli a na kterem ne.
	// 1 = navstivene policko
	// 0 = zatim nenavstivene
	matrix_s *M_visited = matrix_alloc_mem(M->header);
	if(M_visited == NULL)
		return E_MEM;

	// Pocet jiz nalezenych bublin
	unsigned int bubbles_no = 0;
	// Rozmery matice
	int dimension = matrix_get_dimension(M->header);

	// Projdeme vsechna policka matice a otestujeme
	// pristomnost bublin.
	for(int i = 0; i < dimension; i++) {
		if(matrix_bubbles(M, M_visited, i))
			bubbles_no++;
	}

	*result = bubbles_no;
	matrix_free_mem(M_visited);
	return E_OK;
}


// -----------------------------------------------------------------------------
// Nasledujici funkce obsluhuji uzivatelske rozhrani.
// Pro kazdy platny argument programu je jedna funkce.
// -----------------------------------------------------------------------------
int call_help(char **argv) {
	printf("Program je soucasti projektu cislo 3 do predmetu BIZP:");
	printf("\tMaticove operace\n\n");
	printf("Autor: Vlastimil Slintak, 121035@FEKT\n\n");

	printf("Pouziti: %s <parametr> [argument1] [argument2]\n", argv[0]);
	printf("Parametry programu jsou:\n");

	for(int i = 0; i < parameters_no; i++) {
		printf("%s\t%d\t%s\n",
			parameters[i].name,
			parameters[i].args_no,
			parameters[i].help);
	}
	return E_OK;
}

int call_test(char **argv) {
	matrix_s *M = load_data_from_file(argv[2]);
	if(M == NULL)
		return E_FILE;

	matrix_print(M);
	matrix_free_mem(M);
	return E_OK;
}

int call_vadd(char **argv) {
	matrix_s *A = load_data_from_file(argv[2]);
	if(A == NULL)
		return E_FILE;
	matrix_s *B = load_data_from_file(argv[3]);
	if(B == NULL)
		return E_FILE;

	matrix_s *C = matrix_alloc_mem(A->header);
	if(C == NULL)
		return E_MEM;
	
	int return_code = vector_addition(A, B, C);

	if(return_code == E_OK)
		matrix_print(C);

	matrix_free_mem(A);
	matrix_free_mem(B);
	matrix_free_mem(C);

	return return_code;
}

int call_vscal(char **argv) {
	matrix_s *A = load_data_from_file(argv[2]);
	if(A == NULL)
		return E_FILE;
	matrix_s *B = load_data_from_file(argv[3]);
	if(B == NULL)
		return E_FILE;

	int result;
	int return_code = vector_scalar_mul(A, B, &result);
	if(return_code == E_OK)
		printf("%d\n", result);


	matrix_free_mem(A);
	matrix_free_mem(B);
	return return_code;
}

int call_mmult(char **argv) {
	matrix_s *A = load_data_from_file(argv[2]);
	if(A == NULL)
		return E_FILE;
	matrix_s *B = load_data_from_file(argv[3]);
	if(B == NULL)
		return E_FILE;

	matrix_s *C = NULL;
	int return_code = matrix_multiplication(A, B, &C);
	if(return_code == E_OK)
		matrix_print(C);

	matrix_free_mem(A);
	matrix_free_mem(B);
	matrix_free_mem(C);

	return return_code;
}

int call_mexpr(char **argv) {
	// Nacteme data se souboru
	matrix_s *A = load_data_from_file(argv[2]);
	if(A == NULL)
		return E_FILE;
	matrix_s *B = load_data_from_file(argv[3]);
	if(B == NULL)
		return E_FILE;

	matrix_s *C = NULL;
	matrix_s *D = NULL;
	// Vypocitame vyraz A*B, vysledek je v C
	int return_code = matrix_multiplication(A, B, &C);
	if(return_code == E_OK)
		// Pokud predchozi vyraz skoncil bez chyb, pokracujeme
		// dalsim nasobenim C*A, vysledek je v D.
		return_code = matrix_multiplication(A, C, &D);

	// Vytiskneme vysledek jen pokud byl bez chyb.
	if(return_code == E_OK)
		matrix_print(D);

	// Uvolneni pameti
	matrix_free_mem(A);
	matrix_free_mem(B);
	matrix_free_mem(C);
	matrix_free_mem(D);

	return return_code;
}

int call_bubbles(char **argv) {
	// Nacteme data ze souboru
	matrix_s *M = load_data_from_file(argv[2]);
	if(M == NULL)
		return E_FILE;

	unsigned int bubbles_no;
	matrix_count_bubbles(M, &bubbles_no);
	printf("%d\n", bubbles_no);

	matrix_free_mem(M);
	return E_OK;
}

int main(int argc, char **argv) {

	// Navratovy kod celeho programu.
	int return_code = E_CL;

	// Projde vsechny moznosti vstupnich parametru.
	// Pokud uzivatel zadal platne vstupni argumenty,
	// zavola funkci, ktera vykona uzivatelem zadanou funkci
	// a vytiskne vysledek na stdout.
	//
	// Kazda funkce vraci navraovy kod.
	///
	for(int i = 0; i < parameters_no; i++) {
		if((argc-2 == parameters[i].args_no) &&
		  (strcmp(parameters[i].name, argv[1]) == 0)) {
			return_code = (parameters[i].function)(argv);
			break;
		}
	}

	// Pokud mame jiny navratovy kod nez E_OK, vytiskneme
	// prislusne chybove hlaseni. A skoncime.
	if(return_code != E_OK) {
		fprintf(stderr, "CHYBA: %s\n", ERROR_MSG[return_code]);
		return 1;
	}

	return 0;
}
