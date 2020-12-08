#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

//
//define a hash table
//
typedef struct Symbol {
        int address;
        char name[7];
        int SourceLine;
        struct Symbol *next;
} Symbol;

typedef struct OP {
	int hex;
	char name[7];
} OP;

/*
const OP OPCode[ 59 ] = { { 24, "ADD" }, { 88, "ADDF" }, { 144, "ADDR" }, { 64, "AND" }, { 180, "CLEAR" }, { 40, "COMP" }, { 136, "COMPF" }, 
{ 160, "COMPR" }, { 36, "DIV" }, { 100, "DIVF" }, { 156, "DIVR" }, { 196, "FIX" }, { 192, "FLOAT" }, { 244, "HIO" }, { 60, "J" }, 
{ 48, "JEQ" }, { 52, "JGT" }, { 56, "JLT" }, { 72, "JSUB" }, { 0, "LDA" }, { 104, "LDB" }, { 80, "LDCH" }, { 112, "LDF" }, { 8, "LDL" }, 
{ 108, "LDS" }, { 116, "LDT" }, { 4, "LDX" }, { 208, "LPS" }, { 32, "MUL" }, { 96, "MULF" }, { 152, "MULR" }, { 200, "NORM" }, { 68, "OR" },  
{ 216, "RD" }, { 172, "RMO" }, { 76, "RSUB" }, { 164, "SHIFTL" }, { 168, "SHIFTR" }, { 240, "SIO" }, { 236, "SSK" }, { 12, "STA" }, 
{ 120, "STB" }, { 84, "STCH" }, { 128, "STF" }, { 212, "STI" }, { 20, "STL" }, { 124, "STS" }, { 232, "STSW" }, {132, "STT" }, { 16, "STX" }, 
{ 28, "SUB" }, { 92, "SUBF" }, { 148, "SUBR" }, { 176, "SVC" }, { 224, "TD" }, { 248, "TIO" }, { 44, "TIX" }, { 184, "TIXR" }, { 220, "WD" } };
 */

//chop string size_t n off the front of the string 
void chopString( char *str, size_t n ) {
	size_t len = strlen( str );
	if( n > len ) return;
	memmove( str, str + n, len - n + 1 );	
}

//makes hex proper length for Object File
//By concatenating 0s to the front of the hex code
char* properLength( char *hex, int length ) {
	char temp[ length + 1 ];
        while( strlen( hex ) < length ) {
        	memset( temp, 0, sizeof( temp ) );
                temp[0] = '0';
                strcat( temp, hex );
                strcpy( hex, temp );
        }
        memset( temp, 0, sizeof( temp ) );
        return hex;
}

void makeNewTRecord( char (*TLine)[], int LocCtr ) {
	        char hex[7];
		memset( hex, 0, sizeof( hex ));
	       (*TLine)[0] = 'T';
	        sprintf( hex, "%X", LocCtr );
	        strcpy( hex, properLength( hex, 6 ) );
	        strcat( (*TLine), hex );
}

int isValidSymbol( char *symbol ) {
	for( int i = 0; i < strlen( symbol ); i++ ) {
		switch( symbol[i] ){
			case 32:
				printf("\tERROR SYMBOL CONTAINS SPACES\n");
				return -1;
			case 36:
				printf("\tERROR SYMBOL CONTAINS $\n");
				return -1;
			case 33:
				printf("\tERROR SYMBOL CONTAINS !\n");
				return -1;
			case 61:
				printf("\tERROR SYMBOL CONTAINS =\n");
				return -1;
			case 43:
				printf("\tERROR SYMBOL CONTAINS +\n");
				return -1;
			case 45:
				printf("\tERROR SYMBOL CONTAINS -\n");
				return -1;
			case 40:
				printf("\tERROR SYMBOL CONTAINS (\n");
				return -1;
			case 41:
				printf("\tERROR SYMBOL CONTAINS )\n");
				return -1;
			case 64:
				printf("\tERROR SYMBOL CONTAINS @\n");
				return -1;
			default:
				continue;
		}
	}
	return 0;
}

int isValidHex( char *hex ) {
	for( int i = 0; i < strlen( hex ); i++ ) {
        	if( isxdigit( hex[i] ) == 0 ) {
                        return -1;
                }
        }
	return 0;
}

//Checks if character is a valid character constant.
int isValidConstant( char *constant ) {
	for( int i = 0; i < strlen( constant ); i++ ) {
		if( ( constant[i] < 32 )  || ( constant[i] == 127 ) ) {
			return -1;
		}
	}
	return 0;
}  

int isDirective (char *symbol) {
	int result;
	if( strcmp( symbol, "START" ) == 0 ) {
		result = 0;
	} else if( strcmp( symbol, "END" ) == 0 ) {
		result = 0;
	} else if( strcmp( symbol, "BYTE" ) == 0 ) {
                result = 0;
        } else if( strcmp( symbol, "WORD" ) == 0 ) {
                result = 0;
        } else if( strcmp( symbol, "RESB" ) == 0 ) {
                result = 0;
        } else if( strcmp( symbol, "RESW" ) == 0 ) {
                result = 0;
        } else if( strcmp( symbol, "RESR" ) == 0 ) {
                result = 0;
        } else if( strcmp( symbol, "EXPORTS" ) == 0 ) {
                result = 0;
        } else {
		result = -1;
	}
	return result;
}

int isFormatOneOrTwo( char *OPCode ) {
	const char *FormatOneOPCodes[6] = { "FIX", "FLOAT", "HIO", "NORM", "SIO", "TIO" };
	const char *FormatTwoOPCodes[11] = { "ADDR", "CLEAR", "COMPR", "DIVR", "MULR", "RMO", "SHIFTL", "SHIFTR", "SUBR", "SVC", "TIXR" };
	int i;
	for( i = 0; i < 11; i++ ) {
		if( (i < 6) && (strcmp( FormatOneOPCodes[i], OPCode ) == 0 ) ) {
			return 1;
		} 
		if( strcmp( FormatTwoOPCodes[i], OPCode ) == 0 ) {
			return 2;
		}
	}
	return -1;
}

//this function helps when the START directive is declared alongside a Label
int isStart( char *start, char *startAddr, int lineNumber, char lineCopy[1024] ) {
                        char *ptr; //needed for strtol()
			int startAddress;
			if( strcmp( start, "START" ) == 0 ) {
                               	if( isValidHex( startAddr ) == -1 ) {
                                       	printf("\tASSEMBLY ERROR: %s\tLine#%d INVALID ADDRESS @ START\n", lineCopy, lineNumber );
                                        return -1;
                                }
                                if ( lineNumber != 1) {
                                        printf( "\tASSEMBLY ERROR: %s\tLine#%d ERROR START DIRECTIVE SHOULD BE DECLARED FIRST\n", lineCopy, lineNumber );
                                        return -1;
                                }
                                startAddress = strtol( startAddr, &ptr, 16 );
				if ( startAddress < 0 ) {
				        printf( "\tASSEMBLY ERROR: %s\tLine#%d INVALID ADDRESS FOR START DIRECTIVE\n", lineCopy, lineNumber );
                                        return -1;
                                }
				return startAddress;
                        }
			printf("\tASSEMBLY ERROR: %s\tLine#%d MISSING START DIRECTIVE\n", lineCopy, lineNumber );
			return -1;
}

//prints hashtable
void print_table (Symbol *hashtable[]) {
        for ( int i = 0; i < 26; i++ ) {
                if ( hashtable[i] == NULL ) {
                } else {
                        Symbol *temp = hashtable[i] -> next;
                        while (temp != NULL){ 
                         printf("Hash:%i\t%s\t%X\n", i, temp -> name, temp -> address);
                         temp = temp -> next;
                        }
                }
        }
}

int insertIntoSymbolTable( Symbol *hashes[], char *symbol, int address, int lineNumber ) {
	Symbol *current;
	Symbol *temp;
	Symbol *last;
	if( symbol[0] >= 97 && symbol[0] <= 122 ) {
		if( hashes[ symbol[0] - 97 ] == NULL ) {
			hashes[ symbol[0] - 97 ] = malloc( sizeof(Symbol*) );
		}
		current = hashes[ symbol[0] - 97 ];
	} else {
		if( hashes[ symbol[0] - 65 ] == NULL ) {
			hashes[ symbol[0] - 65 ] = malloc( sizeof(Symbol*) );
		}
		current = hashes[ symbol[0] - 65 ];
	}
	temp = malloc( sizeof( Symbol ) );
	strcpy( temp -> name, symbol );
	temp -> address = address;
	temp -> next = NULL;
	temp -> SourceLine = lineNumber;
	while( current != NULL ) {
		if( strcmp( current -> name, symbol ) == 0 ) {
			printf("\tASSEMBLY ERROR: SYMBOL %s ALREADY EXISTS IN SYMBOL TABLE\n", symbol );
			return -1;
		}
		last = current;
		current = current -> next;
	}
	last -> next = malloc( sizeof(Symbol) );
	memmove( last -> next, temp, sizeof(Symbol) );
	return 0;
}

void insertIntoTRecord( FILE *objFile, char (*TLine)[], char (*TObj)[], char *addOn, int LocCtr ) {
	if( strlen( (*TLine) ) == 0 ) {
		makeNewTRecord( TLine, LocCtr ); 
	}	
	char hex[7];
	if((strlen( addOn ) + strlen( (*TObj) ) > 60)) {
		sprintf( hex, "%X", ( strlen( (*TObj) )/2 ) );
		strcpy( hex, properLength(hex, 2) );
		strcat( (*TLine), hex );
		strcat( (*TLine), (*TObj) );
		fputs( (*TLine), objFile );
		fputs( "\n", objFile );
		memset( (*TObj), 0, sizeof(char[61]) );
		memset( (*TLine), 0, sizeof(char[70]) );
		makeNewTRecord( TLine, LocCtr );		
	}
       strcat( (*TObj), addOn );	
}

//search hashtable for symbol
Symbol* search_table ( Symbol *hashTable[], char *symbol ) {
	if( symbol == NULL ) {
		return NULL;
	}
	Symbol *temp;
	if ( symbol[0] >= 97 && symbol[0] <= 122) {
		if( hashTable[ symbol[0] - 97 ] ) {
			temp = hashTable[ symbol[0] - 97 ] -> next;
		} else return NULL;
	} else {
		if( hashTable[ symbol[0] - 65 ] ) {
			temp = hashTable[ symbol[0] - 65 ] -> next;
		} else return NULL;
	}
	while ( temp ) {
		char *string = temp -> name;   
		if( strcmp( string, symbol ) == 0 ) {
			return temp;
		}
		temp = temp -> next;
        }
	return NULL;
}

// make sure OPCode is a legitimate OPCode
int searchOPTable( char *OPCode ) {
       int length = strlen( OPCode );
       char OPCpy[ length + 1 ];
       strcpy( OPCpy, OPCode );
       if ( OPCode[ length - 1] == '\n' ) {
		OPCode = strtok(OPCode, " \n\t\r"); 
		strcpy( OPCpy, OPCode );
	}
       if ( OPCode[0] == '+' ) {
		chopString( OPCpy, 1 ); 
       }
        /*const char* OPCodes[59] = {     "ADD", "ADDF", "ADDR", "AND", "CLEAR", "COMP", "COMPF", "COMPR", "DIV", 
                                        "DIVF", "DIVR", "FIX", "FLOAT", "HIO", "J", "JEQ", "JGT", "JLT", "JSUB", 
                                        "LDA", "LDB", "LDCH", "LDF ", "LDL", "LDS", "LDT", "LDX", "LPS", "MUL", "MULF", 
                                        "MULR", "NORM", "OR", "RD", "RMO", "RSUB", "SHIFTL", "SHIFTR", "SIO", "SSK", "STA", 
                                        "STB ", "STCH", "STF", "STI", "STL", "STS", "STSW", "STT", "STX", "SUB", "SUBF", 
                                        "SUBR", "SVC", "TD", "TIO", "TIX", "TIXR", "WD" };*/

	const OP OPTable[ 59 ] = { { 24, "ADD" }, { 88, "ADDF" }, { 144, "ADDR" }, { 64, "AND" }, { 180, "CLEAR" }, { 40, "COMP" }, { 136, "COMPF" },
				   { 160, "COMPR" }, { 36, "DIV" }, { 100, "DIVF" }, { 156, "DIVR" }, { 196, "FIX" }, { 192, "FLOAT" }, 
				   { 244, "HIO" }, { 60, "J" },{ 48, "JEQ" }, { 52, "JGT" }, { 56, "JLT" }, { 72, "JSUB" }, { 0, "LDA" }, 
				   { 104, "LDB" }, { 80, "LDCH" }, { 112, "LDF" }, { 8, "LDL" }, { 108, "LDS" }, { 116, "LDT" }, { 4, "LDX" }, 
				   { 208, "LPS" }, { 32, "MUL" }, { 96, "MULF" }, { 152, "MULR" }, { 200, "NORM" }, { 68, "OR" }, { 216, "RD" }, 
				   { 172, "RMO" }, { 76, "RSUB" }, { 164, "SHIFTL" }, { 168, "SHIFTR" }, { 240, "SIO" }, { 236, "SSK" }, 
				   { 12, "STA" },{ 120, "STB" }, { 84, "STCH" }, { 128, "STF" }, { 212, "STI" }, { 20, "STL" }, { 124, "STS" }, 
				   { 232, "STSW" }, {132, "STT" }, { 16, "STX" }, { 28, "SUB" }, { 92, "SUBF" }, { 148, "SUBR" }, { 176, "SVC" }, 
				   { 224, "TD" }, { 248, "TIO" }, { 44, "TIX" }, { 184, "TIXR" }, { 220, "WD" } }; 
        //Linear search
        for ( int i = 0; i < 59; i++ ) {
                if( strcmp( OPCpy, OPTable[i].name ) == 0 ) {
                        return OPTable[i].hex;
                }
        }
        return -1;

}



int main( int argc, char *argv[] ) { 

        if ( argc != 2 ) {
                printf("USAGE: %s <filename>\n", argv[0]);
                return -1;
        }

        FILE *inputFile;
        inputFile = fopen(argv[1], "r");
        if ( ! inputFile ) {
                printf("ERROR: Could not open %s for reading\n", argv[1]);
                return -1;
        }

        //Intialize Symbol Table
        Symbol *hashes[26];
        for ( int i = 0; i < 26; i++ ) {
                hashes[i] = NULL;
        }
        char line[1024];
        char *symbol;
        int lineNumber = 1;
        int flag = -1;
        int LocCtr = 0; //the address of the current line
        int startAddress = 0;
	int endCounter = 0; //counts the number of END directives can't be greater than 1.
	int firstInstruc = -1; //get first executable instruction address

	//preload Symbol Table with register values
	insertIntoSymbolTable( hashes, "A", 0, -1000 );
	insertIntoSymbolTable( hashes, "X", 1, -1000 );
 	insertIntoSymbolTable( hashes, "L", 2, -1000 );
	insertIntoSymbolTable( hashes, "B", 3, -1000 );
        insertIntoSymbolTable( hashes, "S", 4, -1000 );
        insertIntoSymbolTable( hashes, "T", 5, -1000 );
        insertIntoSymbolTable( hashes, "F", 6, -1000 );
        insertIntoSymbolTable( hashes, "PC", 8, -1000 );
        insertIntoSymbolTable( hashes, "SW", 9, -1000 );
	
	//Read file line-by-line until EOF
        while( fgets(line, 1024, inputFile) ) {
                char lineCopy[1024];
		strcpy( lineCopy, line );
                //Ignore # for comments in Assembly language
                if ( line[0] == 35 ) {
                        continue;
                }
		
                //Identify a symbol
                if ( ( line[0] >= 65 ) && ( line[0] <= 90) ) {
                         flag = 1;
                }

                //create Symbol table
                char *tokens = strtok(line, " \t\r\n");
		if( tokens == NULL ) {
		//endCounter == 1 means that blank lines are allowed after an END directive
			if( endCounter == 1 ) {
				
			} else {
				printf("\tASSEMBLY ERROR\n\tLine#%d NO BLANK LINES ALLOWED\n", lineNumber);
                        	return -1;
			}
                }
                while ( tokens ) {
                        //read symbol
                        if( flag == 1 ) {
				if ( isValidSymbol( tokens ) == -1 ){
					printf("\tASSEMBLY ERROR: %s\tLine#%d SYMBOL NAME CONTAINS INVALID CHARACTERS\n", lineCopy, lineNumber);
					return -1;
				} 
				//to get the address from START directive if there is one
                                if (lineNumber == 1) {
                                        symbol = tokens;
					char *start = tokens = strtok( NULL, " \t\n\r" );
					char *startAddr = strtok( NULL, " \t\n\r" );
					if( start && startAddr ) { 
						startAddress = LocCtr = isStart( start, startAddr, lineNumber, lineCopy);
					} else {
						printf("\tASSEMBLY ERROR: %s\tLine#%d MISSING START DIRECTIVE OR ADDRESS\n", lineCopy,lineNumber);
						return -1;
					}
                                        if ( startAddress == -1 ) {
						return -1;
                                        } 
                                } else { 
                                        symbol = tokens;
                                }
				if ( isDirective( symbol ) == 0 ) {
					printf("\tASSEMBLY ERROR: %s\tLine#%d SYMBOL IS DIRECTIVE\n", lineCopy, lineNumber );
					return -1;
				}
				if ( ( strlen ( symbol ) ) > 6 ) {
					printf("\tASSEMBLY ERROR: %s\tLine#%d SYMBOL IS LONGER THAN 6 CHARC\n", lineCopy, lineNumber );
                                        return -1;
				}
                                flag = 0;
                                Symbol *current; //pointer to Symbol -> contains Linked List
                                Symbol *temp;
                                Symbol *last;
                                if ( symbol[0] >= 97 && symbol[0] <= 122) {
                                        if ( hashes[ symbol[0] - 97 ] == NULL ) {
                                                hashes[ symbol[0] - 97 ] = malloc( sizeof(Symbol*) );
                                        }
                                        current = hashes[ symbol[0] - 97 ];
                                } else  {
                                        if ( hashes[ symbol[0] - 65 ] == NULL ) {
                                            hashes[ symbol[0] - 65 ] = malloc( sizeof(Symbol*) );     
                                        }

                                        current = hashes[ symbol[0] - 65 ];
                                }
                                temp = malloc( sizeof(Symbol) );
                                strcpy( temp -> name, symbol );
                                temp -> address = LocCtr;
                                temp -> next = NULL;
                                temp -> SourceLine = lineNumber;
                                while ( current != NULL ) {
                                        if ( strcmp( current -> name, symbol ) == 0 ) {
                                                printf("\tASSEMBLY ERROR: %s\tLINE#%d ERROR DUPLICATE SYMBOLS\n", lineCopy, lineNumber);
                                                return -1;
                                        } 
                                        last = current;
                                        current = current -> next;
                                }
                                last -> next = malloc( sizeof(Symbol) );
                                memmove( last -> next, temp, sizeof( Symbol ) );
                                //printf("%s\t%X\n", symbol, LocCtr );
			
				//break out the loop after inserting Label w/ startAddress
				if ( strcmp( tokens, "START") == 0 ) {
					break; 
				}
				tokens = strtok( NULL, " \t\n\r" );
				if( tokens == NULL ) {
                        		printf("\tASSEMBLY ERROR: %s\tLine#%d NO OPCODE AFTER SYMBOL\n", lineCopy, lineNumber);
                        		return -1;
               			 }
				if ( strcmp( tokens, "START") == 0 ) {
                                        printf("\tASSEMBLY ERROR: %s\tLINE#%d START DIRECTIVE NOT ALLOWED HERE\n", lineCopy, lineNumber);
					return -1;
                                }	
                        }
                        
			if( strcmp( tokens, "END" ) == 0 ) {
                                endCounter++;
				if( endCounter > 1 ) {
				 	printf("\tASSEMBLY ERROR: %s\tLINE#%d MULTIPLE END DIRECTIVES NOT ALLOWED\n", lineCopy, lineNumber);
                                        return -1;
				}
				break;
                        } else if( searchOPTable( tokens ) != -1 ) {
				if( firstInstruc == -1 ) firstInstruc = LocCtr;
				if( tokens[0] == '+' ) {
					LocCtr += 4; 
				} else if( isFormatOneOrTwo( tokens ) == 1 ) {
				       LocCtr += 1; 
				} else if( isFormatOneOrTwo( tokens ) == 2 ) {
				       LocCtr += 2; 
				} else {
					LocCtr += 3;
				}	
				if ( LocCtr > 1048576 ) {
                                	printf("\tASSEMBLY ERROR: %s\tLine#%d MEMORY NEEDED ( IS %d ) EXCEEDED 1MB\n", lineCopy, lineNumber, LocCtr );
                                	return -1;
                        	}
                                break;
                        } else if( strcmp( tokens, "WORD" ) == 0 ) {
                             	tokens = strtok( NULL, " \t\n\r" );
				if( tokens == NULL ) {
                                	printf("\tASSEMBLY ERROR: %s\tLine#%d MISSING OPERAND AFTER WORD\n", lineCopy, lineNumber);
                                        return -1;
                                }
				int word = atoi( tokens );
				if( word > 8388608 ) {
                                        printf("\tASSEMBLY ERROR: %s\tLine#%d CONSTANT AFTER WORD DIRECTIVE EXCEEDS MAX WORD SIZE OF 2^23\n", lineCopy, lineNumber);
                                        return -1;
                                }
				LocCtr += 3;
				if ( LocCtr > 1048576 ) {
                                        printf("\tASSEMBLY ERROR: %s\tLine#%d MEMORY NEEDED ( IS %d ) EXCEEDED 1MB\n", lineCopy, lineNumber, LocCtr );
                                        return -1;
                                }
                                break;
                        } else if( strcmp( tokens, "RESW" ) == 0 ) {
                                tokens = strtok( NULL, " \t\n\r");
				if( tokens == NULL ) {
                                        printf("\tASSEMBLY ERROR: %s\tLine#%d MISSING OPERAND AFTER RESW\n", lineCopy, lineNumber);
                                        return -1;
                                }
                                LocCtr += 3 * atoi( tokens );
				if ( LocCtr > 1048576 ) {
                                        printf("\tASSEMBLY ERROR: %s\tLine#%d MEMORY NEEDED ( IS %d ) EXCEEDED 1MB\n", lineCopy, lineNumber, LocCtr );
                                        return -1;
                                }
                                break;
                        } else if( strcmp( tokens, "RESB" ) == 0 ) {
                                tokens = strtok( NULL, " \t\n\r");
				if( tokens == NULL ) {
                                        printf("\tASSEMBLY ERROR: %s\tLine#%d MISSING OPERAND AFTER RESB\n", lineCopy, lineNumber);
                                        return -1;
                                }
                                LocCtr += atoi( tokens );
				if ( LocCtr > 1048576 ) {
                                        printf("\tASSEMBLY ERROR: %s\tLine#%d MEMORY NEEDED ( IS %d ) EXCEEDED 1MB\n", lineCopy, lineNumber, LocCtr );
                                        return -1;
                                }
                                break;
                        } else if( strcmp( tokens, "RESR" ) == 0 ) {
                                LocCtr += 3;
				if ( LocCtr > 1048576 ) {
                                        printf("\tASSEMBLY ERROR: %s\tLine#%d MEMORY NEEDED ( IS %d ) EXCEEDED 1MB\n", lineCopy, lineNumber, LocCtr );
                                        return -1;
                                }
				tokens = strtok( NULL, " \t\n\r");
                                if( tokens == NULL ) {
                                        printf("\tASSEMBLY ERROR: %s\tLine#%d MISSING OPERAND AFTER RESR\n", lineCopy, lineNumber);
                                        return -1;
                                }
                                break;
                        } else if( strcmp( tokens, "EXPORTS" ) == 0 ) {
                                LocCtr += 3;
				if ( LocCtr > 1048576 ) {
                                        printf("\tASSEMBLY ERROR: %s\tLine#%d MEMORY NEEDED ( IS %d ) EXCEEDED 1MB\n", lineCopy, lineNumber, LocCtr );
                                        return -1;
                                }
                                break;
                        } else if( strcmp( tokens, "BYTE" ) == 0 ) {
                                tokens = strtok( NULL, "\t\n\r");
                                if( tokens == NULL ) {
                                        printf("\tASSEMBLY ERROR: %s\tLine#%d MISSING OPERAND AFTER BYTE\n", lineCopy, lineNumber);
                                        return -1;
                                }
                                if( tokens[0] == 'X' && tokens[1] == '\'' ) {
					char *hex = strtok( tokens, "X'" );
					if( isValidHex( hex ) == -1 ) {
                                        	printf("\tASSEMBLY ERROR: %s\tLine#%d INVALID HEX OPERAND IN  BYTE\n", lineCopy, lineNumber);
                                        	return -1;
                                	}
                                        LocCtr += ( strlen( hex ) ) / 2;
                                } else if( tokens[0] == 'C' && tokens[1] == '\'' ) {
					char *constant = strtok( tokens, "C'" );
					if( isValidConstant( constant ) == -1 ) {
						printf("\tASSEMBLY ERROR: %s\tLine#%d INVALID CHARACTER CONSTANT OPERAND\n", lineCopy, lineNumber );
						return -1;
					}
                                        LocCtr += strlen( constant );
                                } else {
					printf("\tASSEMBLY ERROR: %s\tLine#%d INVALID OPERAND AFTER BYTE\n", lineCopy, lineNumber);
                                        return -1;
				}
				if ( LocCtr > 1048576 ) {
                                        printf("\tASSEMBLY ERROR: %s\tLine#%d MEMORY NEEDED ( IS %d ) EXCEEDED 1MB\n", lineCopy, lineNumber, LocCtr );
                                        return -1;
                                }
                                break;
                        } else if( strcmp( tokens, "BASE" ) == 0 ) {
				break;	
			} else if( strcmp( tokens, "NOBASE" ) == 0 ) {
				break;
			} else {
                                printf("\tASSEMBLY ERROR: %s\tLine#%d ERROR INVALID OPCODE OR DIRECTIVE OR BAD FIRST LETTER FOR SYMBOL\n", lineCopy, lineNumber);
                                return -1;
                        }
                        tokens = strtok( NULL, " \t\n\r");  
                }

                lineNumber++;
        }
	if( endCounter == 0 ) printf("\tWARNING END DIRECTIVE MISSING\n");
/*     	
	printf("\nSTART ADDRESS: %X\n", startAddress);
	printf("------------\n");
        printf("HASHTABLE\n");
        printf("------------\n");
        print_table( hashes );
	printf("------------\n");
	printf("PROGRAM LENGTH: %X\n", LocCtr - startAddress );
*/	
	
	//begin Project 2
	int programLength = LocCtr - startAddress;
	LocCtr = startAddress;
	char hex[7]; //max hex is 6 characters long 
	char TLine[70];
	memset(TLine, 0, sizeof( TLine )); 
	memset( hex, 0, sizeof( hex ));
	rewind( inputFile );

	//Create .obj file	
	FILE *objFile;
	char objFileName[20]; //14 is max file name length
	strcpy( objFileName, argv[1] );
	strcat( objFileName, ".obj" );
	objFile = fopen( objFileName, "w" ); 
	
	lineNumber  = 1;
	char TObj[ 61 ]; //keeps object code of T Line capacity at 60 
	memset( TObj, 0, sizeof( TObj ));
	int base = -1000; //stores address for base relative addressing
        
	while( fgets( line, 1024, inputFile ) ) {
		char lineCopy[1024];
		strcpy( lineCopy, line );
		
		//Ignore # for comments in Assembly language
                if ( line[0] == 35 ) {
                        continue;
                }
		char *tokens = strtok(line, " \t\r\n"); 
		while ( tokens ) { 
			if( lineNumber == 1 ) {
				if( search_table( hashes, tokens ) == NULL ) {
					fclose( objFile );
					remove( objFileName );
					printf("\tASSEMBLY ERROR: %s\tLine#%d NO PROGRAM/LIBRARY NAME DETECTED\n", lineCopy, lineNumber );
					return -1;
				}
				char HLine[10] ="H";
				strcat( HLine, tokens );
				while( strlen( HLine ) < 7 ) {
					strcat( HLine, " " );
				}
				fputs( HLine, objFile );
				sprintf( hex, "%X", startAddress);
                                strcpy( hex, properLength( hex, 6 ) );
				fputs( hex, objFile );
				sprintf( hex, "%X", programLength );
                                strcpy( hex, properLength( hex, 6 ) ); 
				fputs( hex, objFile );
				fputs( "\n", objFile );
				break;
			}
			if( searchOPTable( tokens ) != -1 ) {
				char op[ strlen(tokens)+1 ]; 
				strcpy( op, tokens ); //OpCode 
				int opCode = searchOPTable( op );

				//if Format 4
				if( op[0] == '+' ) {
					char buffer[9];  
					char *opr = strtok( NULL, " \t\r\n" ); 
					
					//FORMAT 4: Immediate Addressing Mode
					if( opr && opr[0] == '#' ) {
						opr = strtok(opr, " #"); 
						int immediateOpr;

						//immediate operand can be symbol or integer value
						if( search_table( hashes, opr ) != NULL ) {
							immediateOpr = search_table( hashes, opr ) -> address;
						} else {
							immediateOpr = atoi( opr ); 
							//atoi() returns 0 if it cannot convert str -> int
							if( strcmp( opr, "0" ) != 0 && immediateOpr == 0 ) {
								fclose( objFile );
								remove( objFileName );
								printf("\tASSEMBLY ERROR: %s\tLine#%d INVALID IMMEDIATE OPERAND\n", lineCopy, lineNumber );
								return -1;
							}
						}
						if( immediateOpr > 1048575 ) {
 							fclose( objFile );
							remove( objFileName );
							printf("\tASSEMBLY ERROR: %s\tLine#%d IMMEDIATE OPERAND EXCEEDED FORMAT 4 CAPACITY\n", lineCopy, lineNumber );
						       return -1;	
						}
						opCode += 1;
						immediateOpr += 1048576;
						sprintf( hex, "%X", opCode );
						if( strlen( hex ) == 1 ) sprintf( hex, "0%X", opCode );
						strcpy( buffer, hex );
						sprintf( hex, "%X", immediateOpr );
						strcpy( hex, properLength( hex, 6 ) ); //properLength should be 6 because opCode length is 2. 6 + 2 = 8 for 4 bytes.
						strcat( buffer, hex );
						insertIntoTRecord(objFile, &TLine, &TObj, buffer, LocCtr );
					} 
					//FORMAT 4: Indirect Addressing Mode					
					else if( opr && opr[0] == '@' ) {
						opr = strtok( opr, " @" ); 
						if( search_table( hashes, opr ) == NULL ) {
							fclose( objFile );
							remove( objFileName );
							printf("\tASSEMBLY ERROR: %s\tLine#%d INVALID OPERAND (SYMBOL NOT FOUND IN SYMBOL TABLE OR ,X USED)\n", lineCopy, lineNumber );
							return -1;
						}
						opCode += 2;
						int address = search_table( hashes, opr ) -> address;
						address += 1048576;
						sprintf( hex, "%X", opCode );
						if( strlen( hex ) == 1 ) sprintf( hex, "0%X", opCode );
						strcpy( buffer, hex );
						sprintf( hex, "%X", address );
						strcpy( hex, properLength( hex, 6 ) );
						strcat( buffer, hex );
						insertIntoTRecord( objFile, &TLine, &TObj, buffer, LocCtr );
					}
					//FORMAT 4: Simple Addressing Mode
					else {  
						int address;
						opCode += 3; // n and i bits are turned on to 1
						
						//edge case +RSUB
						if( strcmp( op, "+RSUB" ) == 0 ) {
							sprintf( hex, "%X", opCode );
							strcpy( hex, properLength( hex, 2 ) );
							strcat( hex, "100000" );
							insertIntoTRecord( objFile, &TLine, &TObj, hex, LocCtr );
							LocCtr += 4;
							break;
						}

						opr = strtok( opr, " ," ); 
						if( search_table( hashes, opr ) == NULL ) {
							fclose( objFile );
							remove( objFileName );
							printf("\tASSEMBLY ERROR: %s\tLine#%d INVALID OPERAND SYMBOL NOT FOUND IN SYMBOL TABLE\n", lineCopy, lineNumber );
							return -1;
						}
						address = search_table( hashes, opr ) -> address;
						char *x = strtok( NULL, " ," ); 
						if( x ) {
							address += 9437184;
							sprintf( hex, "%X", opCode );
							if( strlen( hex ) == 1 ) sprintf( hex, "0%X", opCode );
							strcpy( buffer, hex );
							sprintf( hex, "%X", address );
							strcpy( hex, properLength( hex, 6 ) );
							strcat( buffer, hex );
							insertIntoTRecord( objFile, &TLine, &TObj, buffer, LocCtr );
						} else {
							address += 1048576;
							sprintf( hex, "%X", opCode );
							if( strlen( hex ) == 1 ) sprintf( hex, "0%X", opCode );
							strcpy( buffer, hex );
							sprintf( hex, "%X", address );
							strcpy( hex, properLength( hex, 6 ) );
							strcat( buffer, hex );
							insertIntoTRecord( objFile, &TLine, &TObj, buffer, LocCtr );
						}
					}
				      	LocCtr += 4;			
				} 
				//FORMAT 1
				else if( isFormatOneOrTwo( op ) == 1 ) {
					sprintf( hex, "%X", opCode );
					if( strlen( hex ) == 1 ) sprintf( hex, "0%X", opCode );
					insertIntoTRecord( objFile, &TLine, &TObj, hex, LocCtr );
					LocCtr += 1;
				} 
				//FORMAT 2
				else if( isFormatOneOrTwo( op ) == 2 ) {
					char *r1 = strtok(NULL, " ,\r\t\n"); 
					char *r2 = strtok(NULL, " ,\r\t\n"); 

					if( r1 == NULL || search_table( hashes, r1 ) == NULL ) {
						fclose( objFile );
						remove( objFileName );
						printf("\tASSEMBLY ERROR: %s\tLine#%d REGISTER OPERAND NOT SPECIFIED CORRECTLY\n", lineCopy, lineNumber );
						return -1;
					}
				
					if( r2 != NULL && search_table( hashes, r2 ) == NULL ) {
						fclose( objFile );
						remove( objFileName );
						printf("\tASSEMBLY ERROR: %s\tLine#%d REGISTER OPERAND NOT SPECIFIED CORRECTLY\n", lineCopy, lineNumber );
						return -1;
					}
					int r1Num, r2Num = 0;
					char buffer[3];
					r1Num = search_table( hashes, r1 ) -> address;
					if( r2 != NULL ) r2Num = search_table( hashes, r2 ) -> address;
					sprintf( hex, "%X", opCode );
					if( strlen( hex ) == 1 ) sprintf( hex, "0%X", opCode );
					strcpy( buffer, hex );
					sprintf( hex, "%X", r1Num );
					strcat( buffer, hex );
					sprintf( hex, "%X", r2Num );
					strcat( buffer, hex );
					insertIntoTRecord( objFile, &TLine, &TObj, buffer, LocCtr );
					LocCtr += 2;
					
				}
				//Format 3
				else {
					int PC; //PC is one instruction forward from LocCtr
					PC = LocCtr + 3;
					char *opr = strtok( NULL, " \t\r\n" );
					char buffer[7];
					int disp;

					//For use with negative displacements in PC Relative Addressing
					int pBitMask = 0x2FFF;
					
					//FORMAT 3: immediate addressing mode
					if( opr && opr[0] == '#' ) { 
						opr = strtok( opr, " #" ); 
						int immediateOpr;
						
						//immediate operand can be symbol or immediate integer value
						if( search_table(hashes, opr ) != NULL ) {
							immediateOpr = search_table( hashes, opr ) -> address;
							//PC Relative Addressing
							disp = immediateOpr - PC;
							if( disp >= -2048 && disp <= 2047 ) {
								if( disp > 0 ) {
									disp += 8192; //because p bit is turned on
									opCode += 1; //i bit is turned on
									sprintf( hex, "%X", opCode );
									strcpy( buffer, properLength( hex, 2 ) );
									sprintf( hex, "%X", disp );
									strcpy( hex, properLength( hex, 4 ) );
									strcat( buffer, hex );
									insertIntoTRecord( objFile, &TLine, &TObj, buffer, LocCtr );
								} 
								//negative displacement
								else {
									disp = disp & pBitMask;
									opCode += 1;
									sprintf( hex, "%X", opCode );
									strcpy( buffer, properLength( hex, 2 ) );
									sprintf( hex, "%X", disp );
									strcpy( hex, properLength( hex, 4 ) );
									strcat( buffer, hex );
									insertIntoTRecord( objFile, &TLine, &TObj, buffer, LocCtr );
								}	
							} 
							//Base Relative Addressing
							else if( base != -1000 ) {
								disp = immediateOpr - base;
								//throw error if displacement is not within 0 - 4095
								if( disp > 4095 || disp < 0 ) {
									fclose( objFile );
									remove( objFileName );
									printf("\tASSEMBLY ERROR: %s\tLine#%d BASE RELATIVE ADDRESSING NOT POSSIBLE WITH DISPLACEMENT %X\n", lineCopy, lineNumber, disp );
									return -1;
								}
								opCode += 1; //this is because i bit is turned to 1
								disp += 16384; //b bit is turned to 1
								sprintf( hex, "%X", opCode );
								strcpy( buffer, properLength( hex, 2 ) );
								sprintf( hex, "%X", disp );
								strcpy( hex, properLength( hex, 4 ) );
								strcat( buffer, hex );
								insertIntoTRecord( objFile, &TLine, &TObj, buffer, LocCtr );
							} else {
								fclose( objFile );
								remove( objFileName );
								printf("\tASSEMBLY ERROR: %s\tLine#%d PC RELATIVE ADDRESSING NOT AVAILABLE SPECIFY BASE REGISTER FOR BASE RELATIVE ADDRESSING\n", lineCopy, lineNumber );
								return -1;
							}	
							
						} 
						// if immediate opr is an integer value
						else {
							immediateOpr = atoi( opr ); 
							//atoi returns 0 if it cannot convert str -> int
							if( strcmp( opr, "0" ) != 0 && immediateOpr == 0 ) {
								fclose( objFile );
								remove( objFileName );
								printf("\tASSEMBLY ERROR: %s\tLine#%d INVALID IMMEDIATE OPERAND\n", lineCopy, lineNumber );
								return -1;
							}
							if( immediateOpr > 4095 || immediateOpr < 0 ) {
								fclose( objFile );
								remove( objFileName );
								printf("\tASSEMBLY ERROR: %s\tLine#%d IMMEDIATE OPERAND EXCEEDED FORMAT 3 CAPACITY\n", lineCopy, lineNumber );
								return -1;
							}
							opCode += 1;
							sprintf( hex, "%X", opCode );
							strcpy( buffer, properLength( hex, 2 ) );
							sprintf( hex, "%X", immediateOpr );
							strcpy( hex, properLength( hex, 4 ) );
							strcat( buffer, hex );
							insertIntoTRecord( objFile, &TLine, &TObj, buffer, LocCtr );
						}
					} 
					//FORMAT 3: INDIRECT ADDRESSING MODE
					else if( opr && opr[0] == '@') {
						opr = strtok( opr, " @" ); 
						if( search_table( hashes, opr ) == NULL ) {
							fclose( objFile );
							remove( objFileName );
							printf( "\tASSEMBLY ERROR: %s\tLine#%d INVALID SYMBOL OPERAND\n", lineCopy, lineNumber );
							return -1;
						}
						int address = search_table( hashes, opr ) -> address;
						disp = address - PC;
						opCode += 2; //n bit is turned on to 1
						//FORMAT 3: Indirect addressing mode PC relative addressing
						if( disp >= -2048 && disp <= 2047 ) {
							if( disp > 0 ) {
								disp += 8192; //p bit is turned to 1
								sprintf( hex, "%X", opCode );
								strcpy( buffer, properLength( hex, 2 ) );
								sprintf( hex, "%X", disp );
								strcpy( hex, properLength( hex, 4 ) );
								strcat( buffer, hex );
								insertIntoTRecord( objFile, &TLine, &TObj, buffer, LocCtr );
							} else {
								address = address & pBitMask;
								sprintf( hex, "%X", opCode );
								strcpy( buffer, properLength( hex, 2 ) );
								sprintf( hex, "%X", address );
								strcpy( hex, properLength( hex, 4 ) );
								strcat( buffer, hex );
								insertIntoTRecord( objFile, &TLine, &TObj, buffer, LocCtr );
							}	
						}
						//FORMAT 3: Indirect Addressing Mode Base relative addressing
						else if( base != -1000 ) {
							disp = address - base;
							if( disp < 0 || disp > 4095 ) {
								fclose( objFile );
								remove( objFileName );
								printf("\tASSEMBLY ERROR: %s\tLine#%d BASE RELATIVE ADDRESSING NOT POSSIBLE WITH DISPLACEMENT %X\n", lineCopy, lineNumber, disp );
								return -1;
							}
							disp += 16384; //b bit is turned on to 1
						       	sprintf( hex, "%X", opCode );
							strcpy( buffer, properLength( hex, 2 ) );
							sprintf( hex, "%X", disp );
							strcpy( hex, properLength( hex, 4 ) );
							strcat( buffer, hex );
							insertIntoTRecord( objFile, &TLine, &TObj, buffer, LocCtr );	
						} else {
							fclose( objFile );
							remove( objFileName );
							printf("\tASSEMBLY ERROR: %s\tLine#%d PC RELATIVE ADDRESSING NOT AVAILABLE SPECIFY BASE REGISTER FOR BASE RELATIVE ADDRESSING\n", lineCopy, lineNumber );
							return -1;
						}
					} 
					//FORMAT 3: Simple Addressing Mode
					else {
						opCode += 3; //n and i bits are turned on to 1

						//edge case RSUB
						if( strcmp( op, "RSUB" ) == 0 ) {
							sprintf(hex, "%X", opCode );
							strcpy( hex, properLength( hex, 2 ) );
							strcat( hex, "0000" );
							insertIntoTRecord( objFile, &TLine, &TObj, hex, LocCtr );
							LocCtr += 3;
							break;
						}

						opr = strtok( opr, " ," ); 
						char *x = strtok( NULL, " ," ); 
						if( search_table( hashes, opr ) == NULL ) {
							fclose( objFile );
							remove( objFileName );
							printf("\tASSEMBLY ERROR: %s\tLine#%d INVALID OPERAND SYMBOL NOT FOUND IN SYMBOL TABLE\n", lineCopy, lineNumber );
							return -1;		
						}
						int address = search_table( hashes, opr ) -> address; 

						//FORMAT 3:NOT INDEXED ADDRESSING SO SIMPLY SIMPLE ADDRESSING
						if( x == NULL ) {
							disp = 	address - PC;

							//FORMAT 3: SIMPLE ADDRESSING PC RELATIVE
							if( disp >= -2048 && disp <= 2047 ) {
								if( disp > 0 ) {
									disp += 8192;
									sprintf( hex, "%X", opCode );
									strcpy( buffer, properLength( hex, 2 ) );
									sprintf( hex, "%X", disp );
									strcpy( hex, properLength( hex, 4 ) );
									strcat( buffer, hex );
									insertIntoTRecord( objFile, &TLine, &TObj, buffer, LocCtr );
								} 
								//negative PC displacement
								else {
									disp = disp & pBitMask; 
									sprintf( hex, "%X", opCode );
									strcpy( buffer, properLength( hex, 2 ) );
									sprintf( hex, "%X", disp );
									strcpy( hex, properLength( hex, 4 ) );
									strcat( buffer, hex );
									insertIntoTRecord( objFile, &TLine, &TObj, buffer, LocCtr );
								}		
							} 
							//FORMAT 3: SIMPLE ADDRESSING BASE RELATIVE
							else if( base != -1000 ) {
								disp = address - base;
								if( disp < 0 || disp > 4095 ) {
									printf("\tASSEMBLY ERROR: %s\tLine#%d BASE RELATIVE ADDRESSING NOT POSSIBLE WITH DISPLACEMENT %X\n", lineCopy, lineNumber, disp );
									return -1;
								}
								disp += 16384;
								sprintf( hex, "%X", opCode );
								strcpy( buffer, properLength( hex, 2 ) );
								sprintf( hex, "%X", disp );
								strcpy( hex, properLength( hex, 4 ) );
								strcat( buffer, hex );
								insertIntoTRecord( objFile, &TLine, &TObj, buffer, LocCtr );
							} else {
								fclose( objFile );
								remove( objFileName );
								printf("\tASSEMBLY ERROR: %s\tLine#%d PC RELATIVE ADDRESSING NOT AVAILABLE SPECIFY BASE REGISTER FOR BASE RELATIVE ADDRESSING\n", lineCopy, lineNumber );
								return -1;
							}
						} 
						//FORMAT 3: SIMPLE ADDRESSING WITH INDEXED ADDRESSING
						else {
							int pxBitMask = 0xAFFF; //for negative PC disp in INDEXED ADDRESSING
							disp = address - PC;
							//FORMAT 3: INDEXED ADDRESSING MODE WITH PC RELATIVE 
							if( disp >= -2048 && disp <= 2047 ) {
								if( disp > 0 ) {
									disp += 40960; //X and P bit is turned on to 1
									sprintf( hex, "%X", opCode );
									strcpy( buffer, properLength( hex, 2 ) );
									sprintf( hex, "%X", disp );
									strcpy( hex, properLength( hex, 4 ) );
									strcat( buffer, hex );
									insertIntoTRecord( objFile, &TLine, &TObj, buffer, LocCtr );
								}
								//PC negative displacement in FORMAT 3:INDEXED ADDRESSING
								else {
									disp = disp & pxBitMask;
									sprintf( hex, "%X", opCode );
									strcpy( buffer, properLength( hex, 2 ) );
									sprintf( hex, "%X", disp );
									strcpy( hex, properLength( hex, 4 ) );
									strcat( buffer, hex );
									insertIntoTRecord( objFile, &TLine, &TObj, buffer, LocCtr );
								}
							} 
							//FORMAT 3: INDEXED ADDRESSING MODE WITH BASE RELATIVE
							else if( base != -1000 ) {
								disp = address - base;
						       		if ( disp < 0 || disp > 4095 ) {
									fclose( objFile );
									remove( objFileName );
									printf("\tASSEMBLY ERROR: %s\tLine#%d BASE RELATIVE ADDRESSING NOT POSSIBLE WITH DISPLACEMENT %X\n", lineCopy, lineNumber, disp );
									return -1;
								}
								disp += 49152;
								sprintf( hex, "%X", opCode );
								strcpy( buffer, properLength( hex, 2 ) );
								sprintf( hex, "%X", disp );
								strcpy( hex, properLength( hex, 4 ) );
								strcat( buffer, hex );
								insertIntoTRecord( objFile, &TLine, &TObj, buffer, LocCtr );
							} else {
								fclose( objFile );
								remove( objFileName );
								printf("\tASSEMBLY ERROR: %s\tLine#%d PC RELATIVE ADDRESSING NOT AVAILABLE SPECIFY BASE REGISTER FOR BASE RELATIVE ADDRESSING\n", lineCopy, lineNumber );
								return -1;
							}
						}
					}
					LocCtr += 3;
				}		
				break;
			} else {
				if( strcmp( "END", tokens ) == 0 ) {
					if( firstInstruc == -1 ) {
						fclose( objFile );
						remove( objFileName );
						printf("\tASSEMBLY ERROR\n\tTHERE ARE NO INSTRUCTIONS\n");
						return -1;
					}

					tokens = strtok( NULL, " \t\n\r" );
					if( tokens == NULL || search_table( hashes, tokens ) == NULL ) {
						fclose( objFile );
						remove( objFileName );
						printf("\tASSEMBLY ERROR: %s\tLine#%d END DOESN'T HAVE SYMBOL OR SYMBOL NOT FOUND IN SYMBOL TABLE\n", lineCopy, lineNumber );
						return -1;
					}

				       	if( strlen( TLine ) != 0 ) {
						int TSize = strlen( TObj ) / 2;
						sprintf( hex, "%X", TSize );
						strcpy( hex, properLength( hex, 2 ) );
						strcat( TLine, hex );
						strcat( TLine, TObj );
						fputs( TLine, objFile );
						fputs( "\n", objFile );
						memset( TObj, 0, sizeof( TObj ) );
						memset( TLine, 0, sizeof( TLine ) );	
					}	
					sprintf( hex, "%X", firstInstruc );
					strcpy( hex, properLength( hex, 6 ) );
					char end[8];
					memset( end, 0, sizeof( end ));
					end[0] = 'E'; 
					strcat( end, hex ); 
					fputs( end, objFile );
				       	fputs( "\n", objFile );	
				}
				else if( strcmp( "BYTE", tokens ) == 0 ) {
					tokens = strtok( NULL, "\t\n\r");
                                	if( tokens[0] == 'X' ) {
                                        	char *hexConstant = strtok( tokens, "X'" );
						int space = ( strlen( hexConstant ) ) / 2;
						int TLength = strlen( TLine ) + strlen( TObj );
						if( ( TLength + strlen( hexConstant ) )  > 69 || TLength == 0 ) {
							if( TLength != 0 ) {
								int TSize = strlen( TObj ) / 2;
								sprintf( hex, "%X", TSize );
								strcpy( hex, properLength( hex, 2 ) );
								strcat( TLine, hex );
								strcat( TLine, TObj );
								fputs( TLine, objFile );
								fputs( "\n", objFile );
								memset( TObj, 0, sizeof( TObj ) );
								memset( TLine, 0, sizeof( TLine ) );
							}
							TLine[0] = 'T';
							sprintf( hex, "%X", LocCtr );
							strcpy( hex, properLength( hex, 6 ) );
							strcat( TLine, hex );
						}
						if( strlen( hexConstant ) > 60 ) {
					       		while( strlen( hexConstant ) >= 60 ) {
								strncpy( TObj, hexConstant, 60 );
								chopString( hexConstant, 60 );
								int TSize = strlen( TObj ) / 2;
								sprintf( hex, "%X", TSize );
								strcpy( hex, properLength( hex, 2 ) );
								strcat( TLine, hex );
								strcat( TLine, TObj );
								fputs( TLine, objFile );
								fputs( "\n", objFile );
								memset( TObj, 0, sizeof( TObj ) );
								memset( TLine, 0, sizeof( TLine ) );
								LocCtr += 30;
								makeNewTRecord( &TLine, LocCtr );
							}
							if( strlen( hexConstant ) > 0 ) {
								strcpy( TObj, hexConstant );
								LocCtr += ( strlen( TObj ) / 2 );
								memset( hexConstant, 0, sizeof( hexConstant ) );
							}
						} else {
							strcat( TObj, hexConstant );
                                        		LocCtr += space;
						}
					} else {
                                        	char *charConstant = strtok( tokens, "C'" );
						int space = strlen( charConstant );
						int TLength = strlen( TLine ) + strlen( TObj );
						if( ( TLength + ( strlen( charConstant ) * 2 ) )  > 69 || TLength == 0 ) {
							if( TLength != 0 ) {
								int TSize = strlen( TObj ) / 2;
								sprintf( hex, "%X", TSize );
								strcpy( hex, properLength( hex, 2 ) );
								strcat( TLine, hex );
								strcat( TLine, TObj );
								fputs( TLine, objFile );
								fputs( "\n", objFile );
								memset( TObj, 0, sizeof( TObj ) );
								memset( TLine, 0, sizeof( TLine ) ); 
							}
							TLine[0] = 'T';
							sprintf( hex, "%X", LocCtr );
							strcpy( hex, properLength( hex, 6 ) );
							strcat( TLine, hex );
						}
						char hexStr[ strlen( charConstant ) * 2 ] ;
						memset( hexStr, 0, sizeof( hexStr ) );
						for( int i = 0; i < strlen( charConstant ); i++ ) { 
							sprintf( hex, "%X", (int)( charConstant[i] ) ); 
							strcat( hexStr, hex );
						}
						if( strlen( hexStr ) > 60) {
							while( strlen( hexStr ) >= 60 ) {
							       strncpy( TObj, hexStr, 60 );
							       chopString( hexStr, 60 );
							       int TSize = strlen( TObj ) / 2; 
							       sprintf( hex, "%X", TSize );
							       strcpy( hex, properLength( hex, 2 ) );
							       strcat( TLine, hex );
							       strcat( TLine, TObj );
							       fputs( TLine, objFile );
							       fputs( "\n", objFile );
							       memset( TObj, 0, sizeof( TObj ) );
							       memset( TLine, 0, sizeof( TLine ) );
							       LocCtr += 30;
							       makeNewTRecord( &TLine, LocCtr );
							}
							if( strlen( hexStr ) > 0 ) {
								strcpy( TObj, hexStr );
								LocCtr += ( strlen( TObj ) / 2 );
								memset( hexStr, 0, sizeof( hexStr ) );
							}
								
						} else 	{	
							strcat( TObj, hexStr );
                                        		LocCtr += space;
							memset( charConstant, 0, sizeof( charConstant ) );
						}
                                	}
				}
				else if( strcmp( "WORD", tokens ) == 0 ) {
					char *decimalConstant = strtok( NULL, " \t\n\r" );
					int decimal = atoi( decimalConstant );
					sprintf( hex, "%X", decimal );
					strcpy( hex, properLength( hex, 6 ) );
					int TLength = strlen( TLine ) + strlen( TObj );
					if( ( TLength + strlen( hex ) )  > 69 || TLength == 0 ) {
						if( TLength != 0 ) {
							int TSize = strlen( TObj ) / 2;
							sprintf( hex, "%X", TSize );
							strcpy( hex, properLength( hex, 2 ) );
							strcat( TLine, hex );
							strcat( TLine, TObj );
							fputs( TLine, objFile );
							fputs( "\n", objFile );
							memset( TObj, 0, sizeof( TObj ) );
							memset( TLine, 0, sizeof( TLine ) );
						}
						TLine[0] = 'T';
						sprintf( hex, "%X", LocCtr );
						strcpy( hex, properLength( hex, 6 ) );
						strcat( TLine, hex );
					}
					sprintf( hex, "%X", decimal );
					strcpy( hex, properLength( hex, 6 ) );
					strcat( TObj, hex );
					LocCtr += 3;
				}
				else if( strcmp( "RESB", tokens ) == 0 ) {
					int TLength = strlen( TLine ) + strlen( TObj );
					if( TLength > 0 ) {
						int TSize = strlen( TObj ) / 2;
						sprintf( hex, "%X", TSize );
						strcpy( hex, properLength( hex, 2 ) );
						strcat( TLine, hex );
						strcat( TLine, TObj );
						fputs( TLine, objFile );
						fputs( "\n", objFile );
						memset( TObj, 0, sizeof( TObj ) );
						memset( TLine, 0, sizeof( TLine ) );
					}
					tokens = strtok( NULL, " \t\n\r");
					LocCtr += atoi( tokens );
					break;
				}
				else if( strcmp( "RESW", tokens ) == 0 ) {
					int TLength = strlen( TLine ) + strlen( TObj );
					if( TLength > 0 ) {
						int TSize = strlen( TObj ) / 2;
						sprintf( hex, "%X", TSize );
						strcpy( hex, properLength( hex, 2 ) );
						strcat( TLine, hex );
						strcat( TLine, TObj );
						fputs( TLine, objFile );
						fputs( "\n", objFile );
						memset( TObj, 0, sizeof( TObj ) );
						memset( TLine, 0, sizeof( TLine ) );
					}
					tokens = strtok( NULL, " \t\n\r");
					LocCtr += 3 * atoi( tokens );
					break;
				}
				else if( strcmp( "RESR", tokens ) == 0 ) {
					int TLength = strlen( TLine ) + strlen( TObj );
					if( TLength > 0 ) {
						int TSize = strlen( TObj ) / 2;
						sprintf( hex, "%X", TSize );
						strcpy( hex, properLength( hex, 2 ) );
						strcat( TLine, hex );
						strcat( TLine, TObj );
						fputs( TLine, objFile );
						fputs( "\n", objFile );
						memset( TObj, 0, sizeof( TObj ) );
						memset( TLine, 0, sizeof( TLine ) );
					}
					LocCtr += 3;
					break;
				}
				else if( strcmp( "EXPORTS", tokens ) == 0 ) {
					LocCtr += 3;
					break;
				}
				else if( strcmp( "BASE", tokens ) == 0 ) {
					tokens = strtok( NULL, " \n\t\r" ); 
					Symbol *t = search_table( hashes, tokens );
					if( t ) {
						base = t -> address; 
					}
					break;
				}
				else if( strcmp( "NOBASE", tokens ) == 0 ) {
					base = -1000; //base register cannot be used for base relative addressing
				        break;	
				}
			}
			tokens = strtok( NULL, " \t\r\n" );
		}
		lineNumber++;
    }

	fclose( inputFile );
	fclose( objFile ); 
}


