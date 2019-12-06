
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


static storage_t** deliverySystem; 			//storage_t< *storage_t <**deliverySystem: double pointer
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
		printf("<<<<<<<<<<<<<<<<<<<<<<<< : %s >>>>>>>>>>>>>>>>>>>>>>>>>>>>\n", deliverySystem[x][y].context);
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
	deliverySystem[x][y].building=0;
	deliverySystem[x][y].cnt=0;//the cell is empty
	deliverySystem[x][y].context=malloc(100*sizeof(char));//allocate 100chars to context
	deliverySystem[x][y].room=0;
	
	int i;//i is to set ith element of passwd
	for(i=0;i<PASSWD_LEN+1;i++) //set the passwd all 0
	deliverySystem[x][y].passwd[i]='\0';
}

//get password input and check if it is correct for the cell (x,y)
//int x, int y : cell for password check
//return : 0 - password is matching, -1 - password is not matching
static int inputPasswd(int x, int y) {
	char pass[PASSWD_LEN+1];// password just input by user
	scanf("%s",pass);
	if (strcmp(pass,deliverySystem[x][y].passwd)==0){ //compare the password input and the correct passwd
		return 0;
	}
	
	return -1;
}





// ------- API function for main.c file ---------------

//backup the delivery system context to the file system
//char* filepath : filepath and name to write
//return : 0 - backup was successfully done, -1 - failed to backup
int str_backupSystem(char* filepath) {
	FILE *fp;
	fp=fopen(filepath,"w");//open the file in writing mode
	
	if(fp==NULL)
		return -1;
		
	int i,j;
	fprintf(fp,"%d %d\n%s\n",systemSize[0],systemSize[1],masterPassword);// write row, column and masterpassword
	
	/*write the storage information lines*/
	for(i=0;i<systemSize[0];i++){ //i:row #
		for(j=0;j<systemSize[1];j++){//j:column#
			if(deliverySystem[i][j].cnt!=0)//if the cell is not empty
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
	FILE *fp;
	fp=fopen(filepath,"r");//open the file in read mode
	if (fp==NULL)
		return -1;
	
	fscanf(fp,"%d %d\n%s\n",&systemSize[0],&systemSize[1],masterPassword);//read the first two lines as systemsize and masterpassword
	deliverySystem = malloc(systemSize[0]*sizeof(storage_t*));//allocate system row size of storage_t* 
	
	int i,j;
	for (i=0;i<systemSize[0];i++){
		//deliverysystem[i] would be the pointer of ith row storage_t array
		deliverySystem[i]=malloc(systemSize[1]*sizeof(storage_t));//allocate column storage_t to ith row pointer
	}
	
	for(i=0;i<systemSize[0];i++){//i:row #
		for(j=0;j<systemSize[1];j++)//j:column #
			initStorage(i,j);	//initialize all the storages
	}
	
	if (deliverySystem==NULL)
		return -1;

	int row;//row: the row number of each line in the file
	int col;//col: the column number of each line in the file

	char c;//used to check the EOF
	do{
		fscanf(fp,"%d %d",&row,&col);//scanf which row and column to put from the file
		/*scanf storage informations from the file*/
		fscanf(fp,"%d %d %s %s",&deliverySystem[row][col].building,&deliverySystem[row][col].room,deliverySystem[row][col].passwd,deliverySystem[row][col].context);
		deliverySystem[row][col].cnt=1; //the storage cell is filled

	} while((c=fgetc(fp))!=EOF);//until c get EOF
	
	for(i=0;i<systemSize[0];i++){ //i:row#, j:column#
		for(j=0;j<systemSize[1];j++){
			if(deliverySystem[i][j].cnt==1)//if the cell is occupied, storedCnt increases by 1
			storedCnt++;
		}
	}
	
	fclose(fp);
	
	return 0;
}

//free the memory of the deliverySystem 
void str_freeSystem(void) {
	int i,j;//i: row number j: column number of storage 
	for(i=0;i<systemSize[0];i++){//for each storage cells
		for (j=0;j<systemSize[1];j++)
			free(deliverySystem[i][j].context);//free the context of the storage
			free(deliverySystem[i]); //free the storage cells
	}
	free(deliverySystem);//free the storage_t pointer memories
}



//print the current state of the whole delivery system (which cells are occupied and the destination of the each occupied cells)
void str_printStorageStatus(void) {
	int i, j;
	printf("----------------------------- Delivery Storage System Status (%i occupied out of %i )-----------------------------\n\n", storedCnt, systemSize[0]*systemSize[1]);
	
	printf("\t");
	for (j=0;j<systemSize[1];j++)
	{
		printf(" %i\t\t",j);
	}
	printf("\n-----------------------------------------------------------------------------------------------------------------\n");
	
	for (i=0;i<systemSize[0];i++)
	{
		printf("%i|\t",i);
		for (j=0;j<systemSize[1];j++)
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
	
	if (deliverySystem==NULL)
	return -1;
	
	deliverySystem[x][y].building=nBuilding;//put nBuilding to the storage cell
	deliverySystem[x][y].room=nRoom;// put nRoom to the storage cell
	
	strcpy(deliverySystem[x][y].passwd,passwd);//copy the password just got input to the storage cell
	strcpy(deliverySystem[x][y].context,msg);//copy the msg just input to the storage cell
	
	deliverySystem[x][y].cnt=1;// the storage cell is filled
	storedCnt++;// the whole number of occpied cell increases by 1
	
	return 0;
}



//extract the package context with password checking
//after password checking, then put the msg string on the screen and re-initialize the storage
//int x, int y : coordinate of the cell to extract
//return : 0 - successfully extracted, -1 = failed to extract
int str_extractStorage(int x, int y) {
	char pass[PASSWD_LEN+1];//pass: the password the extracting user just input
	printf(" - input password: ");
	scanf("%s",pass);
	
	if (strcmp(pass,deliverySystem[x][y].passwd)==0||strcmp(pass,masterPassword)==0){
	//if the password just input and the correct password are the same,or the user input the master password
	
		printf("messsage: %s",deliverySystem[x][y].context); //print the message from the storage cell
		
		initStorage(x,y);//initialize the extracted cell
		storedCnt--;// the whole number of occpied cell increases by 1
		
		return 0;
		
		}
	
	return -1;
}

//find my package from the storage
//print all the cells (x,y) which has my package
//int nBuilding, int nRoom : my building/room numbers
//return : number of packages that the storage system has
int str_findStorage(int nBuilding, int nRoom) {
	int i,j;//i is row number, and j is column number
	int numberofPackages=0;//the number of packages which the finder has
	for(i=0;i<systemSize[0];i++){//check all the storage cells for the input building and room number 
		for(j=0;j<systemSize[1];j++){
			if(deliverySystem[i][j].building==nBuilding&&deliverySystem[i][j].room==nRoom){//if a cell corresponding is found
				
				printf(" - Found a package in (%d,%d) ",i,j);//print the row and column number of the cell
				numberofPackages++;//the number of package of the finder is increased 1.
			}
		}
		
	}

	
	return numberofPackages;
}
