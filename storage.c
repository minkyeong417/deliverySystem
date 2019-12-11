
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "storage.h"

/* 
  definition of storage cell structure ----
  members :
  int building : building number of the destination
  int room : room number of the destination
  int cnt : number of packages in the cell
  char passwd[] : password setting (4 characters)
  char *context : package context (message string)
*/
typedef struct {
	int building;
	int room;
	int cnt;
	char passwd[PASSWD_LEN+1];
	
	char *context;
} storage_t; //storage cell


static storage_t** deliverySystem; 			//storage_t <  *storage_t <**deliverySystem: double pointer
static int storedCnt = 0;					//number of cells occupied
static int systemSize[2] = {0, 0};  		//row/column of the delivery system
static char masterPassword[PASSWD_LEN+1];	//master password




// ------- inner functions ---------------

//print the inside context of a specific cell
//int x, int y : cell to print the context
static void printStorageInside(int x, int y) {
	printf("\n------------------------------------------------------------------------\n");
	printf("------------------------------------------------------------------------\n");
	if (deliverySystem[x][y].cnt > 0)
		printf(" <<<<<<<<<<<<<<<<<<<<<<<< : %s >>>>>>>>>>>>>>>>>>>>>>>>>>>>\n", deliverySystem[x][y].context);
	else
		printf("<<<<<<<<<<<<<<<<<<<<<<<< empty >>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
		
	printf("------------------------------------------------------------------------\n");
	printf("------------------------------------------------------------------------\n\n");
}

//initialize the storage
//set all the member variable of storage_t as an initial value
//and allocate memory to the context pointer
//int x, int y : cell coordinate to be initialized
static void initStorage(int x, int y) {
	
	deliverySystem[x][y].building = 0;
	
	//the cell is empty
	deliverySystem[x][y].cnt = 0;
	
	deliverySystem[x][y].room = 0;
	
	//int i is to set ith element of passwd 0
	int i;
	
	
	for(i = 0; i < PASSWD_LEN+1; i++) //set the passwd all 0
		deliverySystem[x][y].passwd[i] = '\0';
}

//get password input and check if it is correct for the cell (x,y) or it matches with the masterpassword
//int x, int y : cell for password check
//return : 0 - password is matching, -1 - password is not matching
static int inputPasswd(int x, int y) {
	
	
	//char pass: the password the extracting user just input
	char pass[PASSWD_LEN+1]; 
	printf(" - input password: ");
	scanf("%s",pass);
	
	
	//compare the password input with the correct passwd and masterpassword 
	if (strcmp(pass,deliverySystem[x][y].passwd) == 0 || strcmp(pass,masterPassword) == 0 ){ 
		return 0;
	}
	
	return -1;
}





// ------- API function for main.c file ---------------

//backup the delivery system context to the file system
//char* filepath : filepath and name to write
//return : 0 - backup was successfully done, -1 - failed to backup

int str_backupSystem(char* filepath) {
	
	//FILE *fp points filepath
	FILE *fp;
	
	//open the file in writing mode
	fp = fopen(filepath,"w");
	
	
	//if fp points nothing, then return -1
	if(fp == NULL)
		return -1;
		
	// back up row, column and masterpassword
	fprintf(fp,"%d %d\n%s\n",systemSize[0],systemSize[1],masterPassword);
	
	
	/*back up the storage information */
	
	// int i, j are the cell coordinate to back-up
	int i,j;
	
	for(i = 0; i < systemSize[0]; i++){ 
		for(j = 0; j < systemSize[1]; j++){
			
			
			//if the cell is not empty
			if(deliverySystem[i][j].cnt != 0)
				fprintf(fp,"%d %d %d %d %s %s\n",i,j,deliverySystem[i][j].building,deliverySystem[i][j].room,deliverySystem[i][j].passwd,deliverySystem[i][j].context);	
		}

	}
	
	fclose(fp);
	return 0;
}



//create delivery system on the double pointer deliverySystem
//char* filepath : filepath and name to read config parameters (row, column, master password, past contexts of the delivery system
//return : 0 - successfully created, -1 - failed to create the system
int str_createSystem(char* filepath) {
	
	//FILE *fp points to filepath
	FILE *fp;
	
	//open the file in read mode
	fp = fopen(filepath,"r");
	
	//if fp points nothing, return -1
	if (fp == NULL)
		return -1;
		
	//read the first two lines as systemsize and masterpassword
	fscanf(fp,"%d %d\n%s\n",&systemSize[0],&systemSize[1],masterPassword);
	
	//allocate system row size of storage_t* 
	deliverySystem = (storage_t**) malloc(systemSize[0]*sizeof(storage_t*));
	
	if (deliverySystem==NULL)
		return -1;
		
	
	//int i, j are cell coordinate to be initialized
	int i,j; 
	
	for (i = 0; i < systemSize[0]; i++){
		
		//deliverysystem[i] would be the pointer of ith row storage_t array
		deliverySystem[i] = (storage_t*) malloc(systemSize[1]*sizeof(storage_t));//allocate column storage_t to ith row pointer

	}
	
	
	
	for(i = 0; i < systemSize[0]; i++){
		for(j = 0; j < systemSize[1]; j++)
		
			//initialize all the storages
			initStorage(i,j);	
			
	}
	
	if (deliverySystem == NULL)
		return -1;

	/*read from the third line of the file until you get EOF*/
	
	
	//int row, col : cell coordinate to be read from the file
	int row;
	int col;
	
	// char c is used to check the EOF
	char c;
	do{
		//scan the coordinate to put 
		fscanf(fp,"%d %d",&row,&col);
		
		/*scan storage informations*/
		
		fscanf(fp,"%d %d %s ",&deliverySystem[row][col].building,&deliverySystem[row][col].room,deliverySystem[row][col].passwd);
		
		deliverySystem[row][col].context= (char*) malloc(2*sizeof(char));
		
		if (deliverySystem[row][col].context==NULL)
			return -1;
		
	
		//int context_length : length of the input context;
		int context_length=1;
		while ((c=fgetc(fp)) != '\n'&& c != EOF){
			
			if (context_length>1)
				deliverySystem[row][col].context = (char*) realloc(deliverySystem[row][col].context,(context_length+1)*sizeof(char));
			deliverySystem[row][col].context[context_length-1]=c;
			context_length++;
					
		}
		
		context_length--;
		
		deliverySystem[row][col].context[context_length]='\0';
		//the storage cell is filled
		deliverySystem[row][col].cnt = 1; 

	} while(c != EOF);//until c get EOF
	
	/*count how many cells are occupied in this whole delivery system*/
	//int i,j : cell coordinate to count occupied cells
	
	for(i = 0; i < systemSize[0]; i++){ 
		for(j = 0; j < systemSize[1]; j++){
			
			//if the cell is occupied, storedCnt increases by 1
			if(deliverySystem[i][j].cnt == 1)
			storedCnt++;
		}
	}
	
	fclose(fp);
	
	return 0; 
}

//free the memory of the deliverySystem 
void str_freeSystem(void) {
	
	//int i,j : cell coordinate to free the memories
	 
	int i,j;
	
	//for each storage cells
	
	for(i = 0; i < systemSize[0]; i++){
		for (j = 0; j < systemSize[1]; j++){
			if (deliverySystem[i][j].cnt != 0)
				free(deliverySystem[i][j].context);//free the context of the storag	
		}
		
			
			free(deliverySystem[i]); //free the storage cells
	}
	
	free(deliverySystem);//free the storage_t pointer memories
}



//print the current state of the whole delivery system (which cells are occupied and the destination of the each occupied cells)
void str_printStorageStatus(void) {
	int i, j;
	printf("----------------------------- Delivery Storage System Status (%i occupied out of %i )-----------------------------\n\n", storedCnt, systemSize[0]*systemSize[1]);
	
	printf("\t");
	for (j = 0; j < systemSize[1]; j++)
	{
		printf(" %i\t\t",j);
	}
	printf("\n-----------------------------------------------------------------------------------------------------------------\n");
	
	for (i = 0; i < systemSize[0]; i++)
	{
		printf("%i|\t",i);
		for (j = 0; j < systemSize[1]; j++)
		{
			if (deliverySystem[i][j].cnt > 0)
			{
				printf("%i,%i\t|\t", deliverySystem[i][j].building, deliverySystem[i][j].room);
			}
			else
			{
				printf(" -  \t|\t");
			}
		}
		printf("\n");
	}
	printf("--------------------------------------- Delivery Storage System Status --------------------------------------------\n\n");
}



//check if the input cell (x,y) is valid and whether it is occupied or not
int str_checkStorage(int x, int y) {
	if (x < 0 || x >= systemSize[0])
	{
		return -1;
	}
	
	if (y < 0 || y >= systemSize[1])
	{
		return -1;
	}
	
	return deliverySystem[x][y].cnt;//deliverysystem each cell is storage_t. its member is cnt	
}


//put a package (msg) to the cell
//input parameters
//int x, int y : coordinate of the cell to put the package
//int nBuilding, int nRoom : building and room numbers of the destination
//char msg[] : package context (message string)
//char passwd[] : password string (4 characters)
//return : 0 - successfully put the package, -1 - failed to put
int str_pushToStorage(int x, int y, int nBuilding, int nRoom, char msg[MAX_MSG_SIZE+1], char passwd[PASSWD_LEN+1]) {
	
	if (deliverySystem == NULL)
	return -1;
	
	//put nBuilding to the storage cell
	deliverySystem[x][y].building = nBuilding;
	
	// put nRoom to the storage cell
	deliverySystem[x][y].room = nRoom;
	
	
	//copy the password just got input to the storage cell
	strcpy(deliverySystem[x][y].passwd,passwd);
	
	
	//copy the msg just input to the storage cell
	//array pointer
	

	deliverySystem[x][y].context= (char*) malloc((strlen(msg)+1)*sizeof(char));
	
	strcpy(deliverySystem[x][y].context,msg);
	
	// the storage cell is filled
	deliverySystem[x][y].cnt = 1;
		
	// the number of occpied cells in the whole system increases by 1
	storedCnt++;
	
	return 0; 
}



//extract the package context with password checking
//after password checking, then put the msg string on the screen and re-initialize the storage
//int x, int y : coordinate of the cell to extract
//return : 0 - successfully extracted, -1 = failed to extract
int str_extractStorage(int x, int y) {
	
	
	
	//if the password just input and the correct password are the same,or the user input the master password
	if (inputPasswd(x,y) == 0){
	
		//print the message from the storage cell
		printf("messsage: %s",deliverySystem[x][y].context); 
		
		
		//initialize the extracted cell
		initStorage(x,y);
		
		//free context memory
		
		
		free(deliverySystem[x][y].context);
		
		//the number of occpied cells in the whole system decreases by 1
		storedCnt--;
		
		return 0; 
		
		}
	
	return -1;
}

//find my package from the storage
//print all the cells (x,y) which has my package
//int nBuilding, int nRoom : my building/room numbers
//return : number of packages that the storage system has
int str_findStorage(int nBuilding, int nRoom) {
	
	//int i,j are cell coordinate to check occupied cells
	int i,j;
	
	//int numberofPackages : the number of packages which the finder has
	int numberofPackages = 0; 
	
	//check all the storage cells for the input building and room number 
	for(i = 0; i < systemSize[0]; i++){
		for(j = 0; j < systemSize[1]; j++){
			
			//if a cell corresponding is found
			if(deliverySystem[i][j].building == nBuilding && deliverySystem[i][j].room == nRoom){
				
				//print the row and column number of the cell
				printf(" - Found a package in (%d,%d) ",i,j);
				
				//the number of package of the finder increases by 1
				numberofPackages++;
			}
		}
		
	}

	
	return numberofPackages;
}
