/*
 ============================================================================
 Name        : topology.c
 Author      : Mark Jessell
 Version     : 1.0
 Copyright   : No restrictions
 Description : Voxel-based topology calculations for Noddy models
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>

//import library to resize stack
#ifdef __unix__
 #include <sys/resource.h> //to set stack size
#endif

//define compiler flags
#define ARRAYSIZE 10000
#define SEARCHDISTANCE 2
#define STACKSIZE 1000000000 //1GB stack

//clumsy boolean type
#define BOOL char
#define TRUE 1
#define FALSE 0

typedef struct topology {

   unsigned char code[250];
   int litho; //unique lithology identifier
   int diffage; //the time at which this voxel became a unique topology
   int difftype; //1 if the (lithology) of this voxel has changed
   int numVoxels; //count of all voxels of this type in the topology
   int numVolumes; //the number of different (discrete) volumes with the same initial code
} TOPO;

typedef struct point {
	double x;
	double y;
	double z;
	int npoints;
} POINT;

/*struct topology ***topotrimat(); //3D topology array
struct topology **topobimat();
struct topology *topomat();
struct point *dmat();
unsigned char *cmat();
unsigned char **cbimat();
unsigned char ***ctrimat();
int **ibimat();
int *imat();*/

//fuction defs
void read_header(char *rootname, int *nx, int *ny, int *nz, int *nevents, double *xOff, double *yOff, double *zOff, double *scale);
void read_litho(char *rootname, struct topology ***topo, int nx, int ny, int nz);
void read_codes(char *rootname, struct topology ***topo, int nx, int ny, int nz);
void unique_codes(struct topology ***topo, int *ncodes, struct topology *ucodes, int nx, int ny, int nz, double xOff, double yOff, double zOff, double scale, struct point *centroids, int dvol, int v_thresh);
BOOL match(int x, int y, int z, struct topology ***topo, int lith, char* code);
int grow(int sx, int sy, int sz, int nx, int ny, int nz, struct topology ***topo, char label, int lith, char *code);
void calc_topology(char *rootname, struct topology ***topo, int nx, int ny, int nz, int nevents, struct topology **pairs, int *pairsize, int *npairs);
void gocad_network(char *rootname, struct topology ***topo, struct topology *ucodes, int ncodes, struct topology **pairs, int *pairsize, int npairs, struct point *centroids);
void adjacency_matrices(char *rootname, struct topology ***topo, struct topology *ucodes, int ncodes, struct topology **pairs, int *pairsize, int npairs, struct point *centroids, int nlitho);
void unique_models(char *root, int files, int nlitho); //find topologically unique models

//defs for array allocations
struct topology ***topotrimat(int nrl,int nrh,int ncl,int nch,int nzl,int nzh);
unsigned char ***ctrimat(int nrl,int nrh,int ncl,int nch,int nzl,int nzh);
struct topology **topobimat(int nrl,int nrh,int ncl,int nch);
int **ibimat(int nrl,int nrh,int ncl,int nch);
unsigned char **cbimat(int nrl,int nrh,int ncl,int nch);
unsigned char *cmat(int nrl,int nrh);
struct topology *topomat(int nrl,int nrh);
struct point *dmat(int nrl,int nrh);
int *imat(int nrl,int nrh);
void free_trimat(char ***m,int nrl,int nrh,int ncl,int nch,int nzl,int nzh);
void free_bimat(char **m,int nrl,int nrh,int ncl,int nch);
void free_mat(char **m,int nrl,int nrh);


//args: root number_of_models discrete_volumes
//eg:   "my_model 1 1" to processes one model called  my_model with discrete_volumes enforced
int main(argc, argv)
int argc;
char **argv;
{
     char root[250]; //root: the name of noddy project to look at (minus the file extension): ie. path/my_project
     int litho, files, dvol, v_thresh;
     struct topology ***topo;
     int nevents, nx,ny,nz;
     int ncodes;
     struct topology *ucodes;
     int x,y,z;
     struct point *centroids;
     struct topology **pairs;
     int npairs, *pairsize;
     double xOff,yOff,zOff,scale;
     int n, nlitho;

	 //Resize Stack (Linux Only)
     #ifdef __unix__
         printf("Running topology. Checking stack size.... \n");
         //set stack size
         const rlim_t kStackSize = STACKSIZE;
         struct rlimit rl;
         int result;
         
         result = getrlimit(RLIMIT_STACK, &rl);
         if (result == 0)
         {
           printf("Initial stack size = %d. Increasing to %dMb... \n",rl.rlim_cur,kStackSize);
           if ((int)rl.rlim_cur < (int)kStackSize)
           {
             rl.rlim_cur = kStackSize;
             result = setrlimit(RLIMIT_STACK, &rl);
             if (result != 0) //error
             {
               printf("Error: Could not set stack size. Setrlimit returned result = %d\n",result);
             }
             printf("Stack size changed to %d\n",rl.rlim_cur);
           } else
           {
             printf("Stack size sufficient, not changing. Stack size = %d\n",rl.rlim_cur);
           }
         } else
         {
           printf("Error - Could not retrieve stack information. Stack size not increased.\n");
         }
	 #endif
	 
     if(argc <= 1)
     	{
     		printf("Pynoddy topology calculator.\n  arguments: filename [use_discrete_volumes (0=false,1=true)] [volume threshold]\n");
     		return(0);
     	}
     
     //get first argument [root file]
     sscanf(argv[1],"%s",&root); //root file
     
     //get second argument [ensure discrete volumes]
     if (argc > 2)
        sscanf(argv[2],"%d",&dvol); //1 = ensure discrete volumes, 0 = don't ensure discrete volumes
	 else
        dvol = 1; //default is 1
     
     //get third argument [volume threshold]
     if (argc > 3)
        sscanf(argv[3],"%d",&v_thresh);
     else
        v_thresh=20;
        
     printf("Calculating topology for %s...\n",root);
     fflush(stdout);
    
     //get model information from header file
     read_header(root, &nx,&ny,&nz,&nevents, &xOff,&yOff,&zOff,&scale);
    
     //print model information
     printf("Model information:\n Events=%d\n Grid size = (%d,%d,%d)\n", nevents, nx,ny,nz);
     printf(" Offset=(%lf,%lf,%lf)\n Scale=%lf\n", xOff,yOff,zOff,scale);
     fflush(stdout);
     
     //initialise voxel array
     printf("Allocating voxel array. Requires %dMb RAM... ", (nx * ny * nz * sizeof(TOPO)) / 1000000);
     topo = (TOPO ***) topotrimat(0,nx,0,ny,0,nz); //3d topology voxel model (matrix)
     printf("Done.\n");
     fflush(stdout);
     
     //initialise work arrays
     printf("Allocating work arrays. Requires %dMb RAM... ", (ARRAYSIZE * sizeof(TOPO) + ARRAYSIZE*sizeof(POINT) + ARRAYSIZE * 2 * sizeof(TOPO) + ARRAYSIZE * sizeof(int)) / 1000000);
     ucodes = (TOPO *) topomat(0,ARRAYSIZE); //list of unique topology codes
     centroids=(POINT *) dmat(0,ARRAYSIZE); //list of centroids
     pairs=(TOPO **) topobimat(0,ARRAYSIZE,0,2); //matrix containing topology (pairs (ie. topo unit A contacts of topo unit B)
     pairsize=(int *) imat(0,ARRAYSIZE); //matrix counting number of voxel pairs each topology pair is represented by (ie. 'surface area')
     printf("Done.\n");
     fflush(stdout);
     
     printf("Loading voxel array (lithologies & topology codes)... "); 
     read_litho(root, topo, nx,ny,nz); //load lithologies into topo voxel model
     read_codes(root, topo, nx,ny,nz); //load topology codes int topo voxel model
     printf("Done.\n");
     fflush(stdout);
     
     printf("Loading unique codes... \n\n");
     unique_codes(topo, &ncodes, ucodes, nx,ny,nz, xOff,yOff,zOff,scale, centroids,dvol,v_thresh); //condense 3D voxel model into list of unique topology codes
     printf("Done. %d codes found.\n",ncodes);
     fflush(stdout);
     
     printf("Calculating topology network... ");
     calc_topology(root, topo, nx,ny,nz, nevents, pairs, pairsize, &npairs); //calculate topology pairs/pairsizes from voxel model
     printf("Done.\n");
     fflush(stdout);
     
     printf("Writing output... ");
     gocad_network(root, topo, ucodes, ncodes, pairs, pairsize, npairs, centroids); //write gocad network from calculated topology
     adjacency_matrices(root, topo, ucodes, ncodes, pairs, pairsize, npairs, centroids, nlitho); //write adjacency matrices from calculated topology
     printf("Done.\nCleaning up...");
     fflush(stdout);
     
     //cleanup
     free_trimat((char *) topo,0,nx,0,ny,0,nz);
     free_mat((char *) ucodes,0,ARRAYSIZE);
     free_mat((char *) centroids, 0,ARRAYSIZE);
     free_bimat((char *) pairs, 0,ARRAYSIZE,0,2);
     free_mat((char *) pairsize, 0,ARRAYSIZE);
     
     printf("Finished\n\n");
     exit(0); //return
}

//load information from (new) noddy header file (*.g20)
void read_header(char *rootname, int *nx, int *ny, int *nz, int *nevents, double *xOff, double *yOff, double *zOff, double *scale)
{
	FILE * in;
	char fname[250];
    char  dum1[250],dum2[250];

	sprintf(fname, "%s.g20",rootname);
	//printf("%s %s\n",fname, rootname);

	in=fopen(fname, "r");
    if (in == NULL)
    {
        printf("Error: Could not open noddy header file at %s\n", rootname);
        exit(1); //get outta town
    }
    
    //read file
	fscanf(in,"%s %s %d", &dum1, &dum2, nevents);
	//printf("#events %d\n", *nevents);
	fscanf(in,"%s %d %d %d", &dum1, nx,ny,nz);
	//printf("dim x=%d y=%d z=%d\n",*nx,*ny,*nz);
	fscanf(in,"%s %s %lf %lf %lf %lf", &dum1, &dum2, xOff,  yOff,  zOff,  scale);
}

//load lithology matrix from noddy output file (.g12)
void read_litho(char *rootname, struct topology ***topo, int nx, int ny, int nz)
{
	FILE * in;
	char fname[250];
    int x,y,z;

	sprintf(fname, "%s.g12",rootname);
	in=fopen(fname, "r");
    if (in == NULL)
    {
        printf("Error: Could not open noddy lithology matrix at %s\n", rootname);
        exit(1); //get outta town
    }
    
    //load lithologies
	for(z=0;z<nz;z++)
		for(x=0;x<nx;x++)
			for(y=0;y<ny;y++)
				fscanf(in,"%d",&topo[x][y][z].litho);

}

//load topology codes from topology matrix
void read_codes(char *rootname, struct topology ***topo, int nx, int ny, int nz)
{
	FILE * in;
	char fname[250];
    int x,y,z;

	sprintf(fname, "%s.g21",rootname);
	in=fopen(fname, "r");
    if (in == NULL)
    {
        printf("Error: Could not open noddy topology matrix at %s\nPlease ensure that Noddy.exe was run with the 'TOPOLOGY' flag!", rootname);
        exit(1); //get outta town
    }
	for(z=0;z<nz;z++)
		for(y=0;y<ny;y++)
			for(x=0;x<nx;x++)
                fscanf(in,"%s",&topo[x][y][z].code);
}

//find unique codes in topology matrix
void unique_codes(struct topology ***topo, int *ncodes, struct topology *ucodes, int nx, int ny, int nz, double xOff, double yOff, double zOff, double scale, struct point *centroids,int dvol, int v_thresh)
{

	int x,y,z,n;
    int same;
    
    //initialize unique codes array to default values
	for(n=0;n<ARRAYSIZE;n++)
	{
		ucodes[n].litho=-1;
		centroids[n].x=centroids[n].y=centroids[n].z=0;
		centroids[n].npoints=0;
		sprintf((char *) ucodes[n].code,"*");
	}

	*ncodes=0;

    //old code, that does not ensure topological entities are spatial continuous - use if dvol == 0
    if (dvol == 0){
    //dvol = false, hence topo_codes are not checked for spatial continuity. In rare cases (eg. a folded unit that has been truncated by an unconformity) these
    //units may not be spatially continuous, and hence a single topological unit (incorrectly) represents two separate volumes.
    
        for(z=0;z<nz;z++) //find unique codes
            for(y=0;y<ny;y++)
                for(x=0;x<nx;x++)
                {
                    //add 'a' character to the end of the topo code - ensures compatability with algorithm below that splits volumes
                    //based on spatial continuity into separate volumes ('a', 'b', 'c' etc.)
                    int len = strlen(topo[x][y][z].code);
                    topo[x][y][z].code[len] = 'a'; //append label
                    topo[x][y][z].code[len+1] = '\0'; //add null terminatinon
                    
                    //loop through known codes & see if this one is new
                    for(n=0,same=0;n<*ncodes;n++) 
                        if( match(x,y,z,topo,ucodes[n].litho,ucodes[n].code) == TRUE )//topo[x][y][z].litho==ucodes[n].litho && strcmp(topo[x][y][z].code,ucodes[n].code)==0) //already loaded this code
                        {
                            ucodes[n].numVoxels++; //keep track of number of voxels per code
                            same=1;
                            break;
                        }
                        
                    //this is a new code, add to ucodes list
                    if(same==0)
                    {
                        ucodes[*ncodes].litho=topo[x][y][z].litho;
                        strcpy((char *)ucodes[*ncodes].code,(char *)topo[x][y][z].code);
                        ucodes[*ncodes].numVoxels=1; //this code is (now) represented by 1 voxel

                        printf("added lithology %d and code %s\n", ucodes[*ncodes].litho, ucodes[*ncodes].code);
                        *ncodes=*ncodes+1;
                    }
                }
    } else {
        /*
        dvol == true, hence a 'corrected' algorithm is used. This is essentially the same as above,
        except a flood fill algorithm is used to label discrete volumes as 'a', 'b' 'c' etc. to
        ensure that topological units are spatially continuous (which is not guaranteed in the 
        above implementation, e.g.. if a unconformity divides a folded lithology into separate volumes). 
        
        Unfortunately this algorithm is sensitive to pixelation artefacts (floating pixels). To help avoid 
        this the SEARCHDISTANCE flag can be used to increase the separation between voxels required to consider
        them as different volumes.

        Note that this algorithm can require substantial stack size (>1Gb) as it involves deep recursion. On unix
        based systems stack size is set at the start of main(). On windows systems it needs to be set using compiler
        flags. 
        */

        TOPO *fcodes = (TOPO *) topomat(0,ARRAYSIZE); //list of different codes in the file
        char code[250];

        int nfcodes = 0;
        int numVoxels;

        char c; //counter for labelling different volumes a 'a', 'b' etc.
        for (z=0;z < nz; z++) //loop through the voxels
            for (y=0;y < ny; y++)
                for (x=0;x < nx; x++)
                
                    //the last character of the code is a digit - ie. the voxel has not been visited yet, therefore this is a new code
                    if (isdigit(topo[x][y][z].code[strlen(topo[x][y][z].code) - 1])) {
                        c = 'A'; //reset character counter. Note that this only allows 

                        //has this code been seen before (but not filled as it is part of a new volume)?
                        for(n=0,same=0;n<nfcodes;n++)
                            if(match(x,y,z,topo,fcodes[n].litho,fcodes[n].code) == TRUE)
                            {
                                //yes it has, but it must be a new volume
                                same=1;
                                c += fcodes[n].numVolumes; //increment c by number of previously observed volumes (used to separate different volumes)
                                
                                if (c == '[' | c == '\\' | c == ']' | c == '^' | c == '_' | c =='`') //skip annoying characters ( '\' especially)
                                    c = 'a'; //more than 26 codes already! move to lower case letters...
                                if (c == '{') //to many separate volumes - throw an error.
                                {
                                    printf("Error: volume codes full - lithology is divided into more than 52 separate volumes. Try increasing the search distance or volume threshold.\n");
                                    exit(1); //getta outta here
                                }
                                
                                fcodes[n].numVolumes++; //this is a new volume - keep track of how many have been observed
                                break;
                            }
                            
                        //code hasn't been seen before, so add it to the list
                        if (same != 1)
                        {
                            fcodes[nfcodes].litho=topo[x][y][z].litho;
                            strcpy((char *)fcodes[nfcodes].code,(char *)topo[x][y][z].code);
                            fcodes[nfcodes].numVolumes=1;
                            nfcodes++; //number of codes observed
                        }

                        //make a copy of code
                        strcpy(code,(char *)topo[x][y][z].code);

                        //Flood fill adjacent voxels with the same code 
                        numVoxels = grow(x, y, z, nx, ny, nz, topo, c, topo[x][y][z].litho,code);
                        
                        //if volume is under the threshold value, assign as null pixels
                        if (numVoxels < v_thresh)
                        {
                            //update code to flood fill
                            strcpy(code,(char *)topo[x][y][z].code); 
                            
                            //append a ^ character to indicate that these voxels should be ignored
                            grow(x, y, z, nx, ny, nz, topo,'^', topo[x][y][z].litho, code);
                            
                            //this wasn't a real volume so reduce volume count
                            fcodes[n].numVolumes--;
                        } 
                        
                        //add this code to ucode
                        ucodes[*ncodes].litho=topo[x][y][z].litho;
                        strcpy((char *)ucodes[*ncodes].code,(char *)topo[x][y][z].code);
                        ucodes[*ncodes].numVoxels=numVoxels;

                        printf("added new lithology %d and code %s with %d voxels. ID = %d \n", ucodes[*ncodes].litho,ucodes[*ncodes].code,ucodes[*ncodes].numVoxels,*ncodes);
                        *ncodes=*ncodes + 1;
                    }

        free_mat((char *) fcodes,0,ARRAYSIZE); //free fcodes
    }
    
    //find centroids of unique codes
	for(z=0;z<nz;z++) 
		for(y=0;y<ny;y++)
			for(x=0;x<nx;x++)
				for(n=0,same=0;n<*ncodes;n++)
                    //add voxel locations
					if(topo[x][y][z].litho==ucodes[n].litho && strcmp(topo[x][y][z].code,ucodes[n].code)==0)
					{
						centroids[n].x+=(x*scale)+xOff;
						centroids[n].y+=(y*scale)+yOff;
						centroids[n].z+=(scale*(nz-1))-(z*scale)+zOff;
						centroids[n].npoints++;
						break;
					}

    //average voxel locations
	for(n=0;n<*ncodes;n++)
	{
		centroids[n].x=centroids[n].x/centroids[n].npoints;
		centroids[n].y=centroids[n].y/centroids[n].npoints;
		centroids[n].z=centroids[n].z/centroids[n].npoints;
	}

}

BOOL match(int x, int y, int z, struct topology ***topo, int lith, char* code)
{
    if (topo[x][y][z].litho == lith && strcmp(topo[x][y][z].code,code)==0)
        return TRUE;
    return FALSE;
}

/*
A pretty basic recursive flood fill algorithm that appends the specified character (label) to contiguous voxels.

The function itself returns zero if it is called on a voxel with a different code, otherwise
it appends the label character to the voxel code and calls grow(...) for all it's neighbours. It then 
returns 1 + the sum of the results. Note that for big volumes this function can recurse deeply, requiring substantial 
stack space (as the memory is not dynamically allocated it does not go on the heap).
*/
int grow(int sx, int sy, int sz, int nx, int ny, int nz, struct topology ***topo, char label, int lith, char *code)
{
    if (match(sx,sy,sz,topo,lith,code) == TRUE) //if the voxel matches
    {
        int count = 1; //number of voxels changed

        //change code
        int len = strlen(topo[sx][sy][sz].code);
        topo[sx][sy][sz].code[len] = label; //append label
        topo[sx][sy][sz].code[len+1] = '\0'; //add null terminatinon

        //if (verbose == 1)
        //{
           // printf("x=%d y=%d z=%d len=%d lith=%d code=%s\n",sx,sy,sz,len,lith,code);
        //}

        //grow adjacent cells
        int x, y, z;
        for (x = sx-SEARCHDISTANCE; x <= sx+SEARCHDISTANCE; x++)
            for (y = sy-SEARCHDISTANCE; y <= sy+SEARCHDISTANCE; y++)
                for (z = sz-SEARCHDISTANCE; z <= sz+SEARCHDISTANCE; z++)
                {
                    if (x >= 0 && y >= 0 && z >= 0 && x < nx && y < ny && z < nz) //valid coordinates
                        if (!(x == sx && y == sy && z == sz)) //and not the starting coordinates
                            count += grow(x,y,z,nx,ny,nz,topo,label,lith,code);
                }
                
        return count;
    }
    return 0; //no match
}

/*
Builds a topology network (list of edges) containing all the adjacency relationships within the noddy model. Essentially loops through all the voxels,
compares them to each neighbour and adds an edge (between the voxel's code and it's neighbour's code) when the codes are different.
*/
void calc_topology(char *rootname, struct topology ***topo, int nx, int ny, int nz, int nevents, struct topology **pairs, int *pairsize, int *npairs)
{
	FILE * out;
	char fname[250];
	int x,y,z;
	int n, same;
    int intcode1,intcode2, order;
    int samecode;
    int codeseq1,codeseq2;
    
    int litho, n_litho; //lithology of each voxel being checked
    char code[250], n_code[250]; //code of each voxel being checked
    
    //initialize default values
	for(n=0;n<ARRAYSIZE;n++)
	{
		pairs[n][0].litho=-1;
		strcpy((char *) pairs[n][0].code,"*");

		pairs[n][1].litho=-1;
		strcpy((char *) pairs[n][1].code,"*");
		pairsize[n]=0;
	}
	*npairs=0;

    //loop through all voxels
    for(x=0;x<nx-1;x++)
		for(y=0;y<ny-1;y++)
			for(z=0;z<nz-1;z++)
            {
                //get the litho & code for this voxel
                litho = topo[x][y][z].litho;
                strcpy(code,(char *)topo[x][y][z].code);
                
                //compare this voxel with its immediate neighbours (including diagonally)
                int n_x, n_y, n_z;
                for (n_x = x-1; n_x <= x+1; n_x++)
                    for (n_y = y-1; n_y <= y+1; n_y++)
                        for (n_z = z-1; n_z <= z+1; n_z++)
                        {
                            if (n_x >= 0 && n_y >= 0 && n_z >= 0 && n_x < nx && n_y < ny && n_z < nz) //valid coordinates (NB. n_x = neighbour_x, nx = x dimension of array... sorry)
                                if (!(n_x == x && n_y == y && n_z == z)) //and not the starting coordinates
                                    
                                    //compare codes
                                    if(match(n_x,n_y,n_z,topo,litho,code) == FALSE) //codes are different
                                    {
                                        //get neighbour lithology and code
                                        n_litho = topo[n_x][n_y][n_z].litho;
                                        strcpy(n_code,(char *)topo[n_x][n_y][n_z].code);
                                        
                                        //assume this is a new code, and that we should add it
                                        same = 0;
                                        
                                        //check to see if either voxels has a null code
                                        if (code[strlen(code)-1]=='^' || n_code[strlen(n_code)-1]=='^')
                                            same = 1; //a valid pair cannot involve a null code
                                       
                                        
                                        //see if this pair has already been observed
                                        if (same==0)
                                            for (n=0;n<*npairs;n++)
                                            {
                                                if((litho==pairs[n][0].litho && strcmp(code,pairs[n][0].code)==0 && //are they the same?
                                                    n_litho==pairs[n][1].litho && strcmp(n_code,pairs[n][1].code)==0) ||
                                                   (n_litho==pairs[n][0].litho && strcmp(n_code,pairs[n][0].code)==0 &&
                                                    litho==pairs[n][1].litho && strcmp(code,pairs[n][1].code)==0))
                                                {
                                                    same = 1; //this isn't a new pair, don't add it.
                                                    pairsize[n]=pairsize[n]+1; //increment size of this pair (think 'surface area')
                                                    break; //all done here
                                                }
                                            }
                                                                                
                                        //this is a new pair, so add it to the list
                                        if (same == 0)
                                        {
                                            intcode1=atoi(code); //calculate integer topology codes (mainly for export to GoCad)
                                            intcode2=atoi(n_code);
                                            
                                            
                                            //note to self: do I care about the following?? It could be calculated later in python if necessary.
                                            for(n=nevents-1,samecode=0;n>-1;n--) //loop through all events
                                                if(code[n]!=n_code[n]) //and find the (first) event where the topology codes are different
                                                {
                                                    pairs[*npairs][0].diffage=n+1; //set the number of times (events) that this voxel is different to its neighbour

                                                    codeseq1=code[n]-'0'; //wtf does this do??
                                                    codeseq2=n_code[n]-'0';
                                                    
                                                    if(codeseq1 > codeseq2)
                                                        pairs[*npairs][0].difftype=codeseq1;
                                                    else
                                                        pairs[*npairs][0].difftype=codeseq2;

                                                    samecode=1; //these voxels have different topology codes
                                                    break;
                                                }
                                                
                                            if(samecode==0) //these voxels have identical topology codes (but different stratigraphic codes): aka - they're stratigraphic contacts
                                            {
                                                pairs[*npairs][0].diffage=0; //topologically identical (but lithologically different)
                                                pairs[*npairs][0].difftype=0; //topologically identical (but lithologically different)
                                            }
                                            
                                            //save the edge
                                            //write vertex 1 (this voxel)
                                            pairs[*npairs][0].litho=litho;
                                            strcpy((char *) pairs[*npairs][0].code,code);
                                            
                                            //write vertex 2 (the neighbour)
                                            pairs[*npairs][1].litho=n_litho;
                                            strcpy((char *) pairs[*npairs][1].code,n_code);
                                            
                                            pairsize[*npairs]=1; //this pair has been observed once
                                            *npairs=*npairs+1; //increment length of pair array
                                            
                                            //printf("%d\t%d_%s\t%d_%s\t%d\n",n,pairs[n][0].litho,pairs[n][0].code,pairs[n][1].litho,pairs[n][1].code,pairsize[n]);
                                            
                                            if(*npairs >= ARRAYSIZE - 1) //array full
                                            {
                                                printf("Error: Pair array cannot store more than %d elements. Try increasing ARRAYSIZE and recompile...\n",ARRAYSIZE);
                                                exit(1); //getta outta here
                                            }
                                        } 
                                    }
                        }
            }
                
	printf("Found %d adjacency relationships from %d events\n",*npairs, nevents);
    
    //save network
	sprintf(fname, "%s.g23",rootname);
	out=fopen(fname, "w");

	for(n=0;n<*npairs;n++)
		fprintf(out,"%03d_%s\t%03d_%s\t%d\t%d\t%d\n",pairs[n][0].litho,pairs[n][0].code,pairs[n][1].litho,pairs[n][1].code,pairs[n][0].difftype,pairs[n][0].diffage,pairsize[n]);
		//fprintf(out,"%03d_%s\t%03d_%s\n",pairs[n][0].litho,pairs[n][0].code,pairs[n][1].litho,pairs[n][1].code);

	fclose(out);
    
}

void gocad_network(char *rootname, struct topology ***topo, struct topology *ucodes, int ncodes, struct topology **pairs, int *pairsize, int npairs, struct point *centroids)
{
	FILE *out_pl,*out_vs;
	char fname_pl[250], fname_vs[250];
    int n,m;
    int pair0=0,pair1=0;


    sprintf(fname_pl,"%s_p.pl",rootname);
    out_pl=fopen(fname_pl,"w");
    sprintf(fname_vs,"%s_v.vs",rootname);
    out_vs=fopen(fname_vs,"w");

    fprintf(out_pl,"GOCAD PLine 1\nHEADER {name: %s_p\n}\nPROPERTIES DiffType DiffAge Num \nPROPERTY_CLASSES DiffType DiffAge Num\n\n",rootname);
    fprintf(out_vs,"GOCAD Vset 1\nHEADER {name: %s_v\n}\nPROPERTIES Litho  Topo  Number\nPROPERTY_CLASSES Litho  Topo  Number\n\n",rootname);

    for(n=0;n<ncodes;n++) // write out vertices
    {
        if (ucodes[n].code[strlen(ucodes[n].code)-1]!='^') //ignore null vertices
            fprintf(out_vs,"PVRTX %d %lf %lf %lf %d %s %d\n",n+1,centroids[n].x,centroids[n].y,centroids[n].z,ucodes[n].litho, ucodes[n].code, centroids[n].npoints);
    }

    for(n=0;n<npairs;n++)
    {
  	 for(m=0;m<ncodes;m++) //find unique codes to match to pair vertices
  	  {
  		 if(pairs[n][0].litho==ucodes[m].litho && strcmp(pairs[n][0].code,ucodes[m].code)==0)
  			  	  pair0=m;
  		 if(pairs[n][1].litho==ucodes[m].litho && strcmp(pairs[n][1].code,ucodes[m].code)==0)
  			  	  pair1=m;
  	  }

  	  fprintf(out_pl,"ILINE\n");
   	  fprintf(out_pl,"PVRTX 1 %lf %lf %lf %d %d %d\n",centroids[pair0].x,centroids[pair0].y,centroids[pair0].z,pairs[n][0].difftype,pairs[n][0].diffage,pairsize[n]);
   	  fprintf(out_pl,"PVRTX 2 %lf %lf %lf %d %d %d\n",centroids[pair1].x,centroids[pair1].y,centroids[pair1].z,pairs[n][0].difftype,pairs[n][0].diffage,pairsize[n]);
   	  fprintf(out_pl,"SEG 1 2\n");

    }

    fprintf(out_pl,"END\n");
	fprintf(out_vs,"END\n");

    fclose(out_pl);
    fclose(out_vs);
}
/*
 * codes
 * 		2,7,8=fault=0x02
 * 		3=unc=0x01
 * 		5=dyke/plug=0x04
 * 		0=base strat=0x08
 */
void adjacency_matrices(char *rootname, struct topology ***topo, struct topology *ucodes, int ncodes, struct topology **pairs, int *pairsize, int npairs, struct point *centroids, int nlitho)
{
	FILE *out_am,*out_amn;
	char fname_am[250], fname_amn[250];
    int n,xlitho,ylitho;
    int **amn;
    unsigned char **am_code;

    sprintf(fname_am,"%s.g24",rootname);
    out_am=fopen(fname_am,"wb");
    sprintf(fname_amn,"%s.g25",rootname);
    out_amn=fopen(fname_amn,"w");


    amn=(int **) ibimat(0,nlitho+1,0,nlitho+1);
    am_code=(int **) cbimat(0,nlitho+1,0,nlitho+1);


    for(xlitho=1;xlitho<nlitho+1;xlitho++)
    	for(ylitho=1;ylitho<nlitho+1;ylitho++)
    		amn[xlitho][ylitho]=am_code[xlitho][ylitho]=0;


    for(xlitho=1;xlitho<nlitho+1;xlitho++)
    	for(ylitho=1;ylitho<nlitho+1;ylitho++)
    	{
    		for(n=0;n<npairs;n++)
    	     {
    	    	  if(pairs[n][0].litho==xlitho && pairs[n][1].litho==ylitho)
    	    	  {
    	    		  if(pairs[n][0].difftype == 2 || pairs[n][0].difftype == 7 || pairs[n][0].difftype == 8 ) //fault
    	    			  am_code[xlitho][ylitho]=am_code[ylitho][xlitho]=am_code[xlitho][ylitho] | 0x02;
    	    		  else if(pairs[n][0].difftype == 3 )                                                      //unc
    	    			  am_code[xlitho][ylitho]=am_code[ylitho][xlitho]=am_code[xlitho][ylitho] | 0x01;
    	    		  else if(pairs[n][0].difftype == 5 )                                                      //plug/dyke
    	    			  am_code[xlitho][ylitho]=am_code[ylitho][xlitho]=am_code[xlitho][ylitho] | 0x04;
    	    		  else                                                                                     // base strat
    	    			  am_code[xlitho][ylitho]=am_code[ylitho][xlitho]=am_code[xlitho][ylitho] | 0x08;


    	    		  amn[xlitho][ylitho]+=pairsize[n];
    	    		  if(xlitho !=ylitho)
    	    			  amn[ylitho][xlitho]+=pairsize[n];
    	    	  }

    	     }
    	}

    for(xlitho=1;xlitho<nlitho+1;xlitho++)
    {
    	for(ylitho=1;ylitho<nlitho+1;ylitho++)
    	{
    		//printf("%d\t",amn[xlitho][ylitho]);
 			fprintf(out_amn,"%04d\t",amn[xlitho][ylitho]);
 			fwrite(&(am_code[xlitho][ylitho]), sizeof(char), 1, out_am);

    	}

		//printf("\n");
		fprintf(out_amn,"\n");
    }

	fclose(out_amn);
 	fclose(out_am);

    free_bimat(amn, 0,nlitho+1,0,nlitho+1);
    free_bimat(am_code, 0,nlitho+1,0,nlitho+1);

}


void unique_models(char *root, int files, int nlitho) //find topologically unique models
{
	int n,m,uniquemodels,x,y,same;
    char rootname[250],*model;
    FILE *in,*out;
    unsigned char **am,**uam, *cam,*dam;  //one adj mat; unique adj mat; common adj mat; diff adj mat


    am = (unsigned char **) cbimat(0,files,0,nlitho*nlitho);
    uam = (unsigned char **) cbimat(0,files,0,nlitho*nlitho);
    cam = (unsigned char *) cmat(0,nlitho*nlitho);
    dam = (unsigned char *) cmat(0,nlitho*nlitho);

	for(n=1;n<files+1;n++)
	{
    	sprintf(rootname,"%s_%04d.g24",root,n);
    	in=fopen(rootname,"rb");
    	fread(&(am[n-1][0]), sizeof(char), nlitho*nlitho, in);
    	//printf("%d %s %d %s\n",nlitho,rootname,n,&(am[n]));
        fclose(in);
	}


	for(n=0,uniquemodels=0;n<files;n++)
	{
		same=0;
		for(m=0;m<uniquemodels;m++)
			if(memcmp(&(am[n][0]),&(uam[m][0]),nlitho*nlitho)==0)
			{
				same=1;
				break;
			}
		if(same==0)
		{
			memcpy(&(uam[uniquemodels][0]),&(am[n][0]),nlitho*nlitho);
			uniquemodels++;
		}
	}
	printf("unique models %d\n",uniquemodels);
	sprintf(rootname,"%s_uam.bin",root);
	out=fopen(rootname,"wb");

	for(m=0;m<uniquemodels;m++)
		fwrite(&(uam[m][0]), sizeof(char), nlitho*nlitho, out);

	fclose(out);

	memcpy(&(cam[0]),&(uam[0][0]),nlitho*nlitho);

	for(x=0;x<nlitho*nlitho;x++)
	{
		for(m=1,same=1;m<uniquemodels;m++)
		{
			if(uam[m][x]!=cam[x])
			{
				same=0;
				break;
			}
		}
		if(same==1)
		{
			dam[x]='\0';
		}
		else
		{
			cam[x]='\0';
			dam[x]='\1';

		}
	}

	sprintf(rootname,"%s_cam.bin",root);
	out=fopen(rootname,"wb");
	fwrite(&(cam[0]), sizeof(char), nlitho*nlitho, out);
	fclose(out);

	sprintf(rootname,"%s_dam.bin",root);
	out=fopen(rootname,"wb");
	fwrite(&(dam[0]), sizeof(char), nlitho*nlitho, out);
	fclose(out);



    free_bimat(am, 0,files,0,nlitho*nlitho);
    free_bimat(uam, 0,files,0,nlitho*nlitho);
    free_mat(cam,0,nlitho*nlitho);
    free_mat(dam,0,nlitho*nlitho);

}

/************************************************************************
*                          *
*  topology ***topotrimat(nrl,nrh,ncl,nch,nzl,nzh)         *
*                          *
*  int nrl;    index of lowest row in array     *
*  int nrh;    index of highest row in array    *
*  int ncl;    index of lowest col in array     *
*  int nch;    index of highest col in array    *
*  int nzl;    index of lowest layer in array      *
*  int nzh;    index of highest layer in array     *
*                          *
*  dtrimat function allocates non-relocatable double memory 3D array*
*                          *
*  dtrimat() takes 6 arguments               *
*  dtrimat() returns pointer to pointer to pointer to memory allocated*
*                          *
************************************************************************/
struct topology ***topotrimat(nrl,nrh,ncl,nch,nzl,nzh)
int nrl,nrh,ncl,nch,nzl,nzh;
{
   register int i,j;
   struct topology ***m;
   printf("allocating array of %d elements that are %d bytes each. Requires %dMb \n", nrh*nch*nzh, sizeof(struct topology), (nrh*nch*nzh*sizeof(struct topology)) / 1000000);

   m=(struct topology ***) malloc((size_t) (nrh-nrl+1)*sizeof(struct topology**));
   if (!m) {
                 printf("allocation failure 1 in topotrimat()\n");
				 exit(0);
                 return(0);
                }
       else
         {
      m -= nrl;


      for(i=nrl;i<=nrh;i++) {
      m[i]=(struct topology **) malloc((size_t) (nch-ncl+1)*sizeof(struct topology*));
      if (!m[i]) printf("allocation failure 2 in topotrimat()\n");
      m[i] -= ncl;
      for(j=ncl;j<=nch;j++) {
         m[i][j]=(struct topology *) malloc((size_t) (nzh-nzl+1)*sizeof(struct topology));
         if (!m[i][j]) {
			 printf("allocation failure 3 in topotrimat()\n");
			 printf("i=%d j=%d nrl=%d nrh=%d ncl=%d nch=%d nzl=%d nzh=%d\n",i,j,nrl,nrh,ncl,nch,nzl,nzh);
			 exit(0);
		 }
         m[i][j] -= nzl;
      }
      }

      return m;
       }
}
//allocates a 3D character matrix (see above for arg defs)
unsigned char ***ctrimat(nrl,nrh,ncl,nch,nzl,nzh)
int nrl,nrh,ncl,nch,nzl,nzh;
{
   register int i,j;
   unsigned char ***m;

   m=(unsigned char ***) malloc((size_t) (nrh-nrl+1)*sizeof(unsigned char**));
   if (!m) {
                 printf("allocation failure 1 in ctrimat()\n");
                 return(0);
                }
       else
         {
      m -= nrl;


      for(i=nrl;i<=nrh;i++) {
      m[i]=(unsigned char **) malloc((size_t) (nch-ncl+1)*sizeof(unsigned char*));
      if (!m[i]) printf("allocation failure 2 in ctrimat()\n");
      m[i] -= ncl;
      for(j=ncl;j<=nch;j++) {
         m[i][j]=(unsigned char *) malloc((size_t) (nzh-nzl+1)*sizeof(unsigned char));
         if (!m[i][j]) printf("allocation failure 3 in ctrimat()\n");
         m[i][j] -= nzl;
      }
      }

      return m;
       }
}
//2d topology matrix
struct topology **topobimat(nrl,nrh,ncl,nch)
int nrl,nrh,ncl,nch;
{
   register int i,j;
   struct topology **m;

   m=(struct topology **) malloc((size_t) (nrh-nrl+1)*sizeof(struct topology*));
   if (!m) {
                 printf("allocation failure 1 in topobimat()\n");
                 return(0);
                }
       else
         {
      m -= nrl;


      for(i=nrl;i<=nrh;i++) {
      m[i]=(struct topology *) malloc((size_t) (nch-ncl+1)*sizeof(struct topology));
      if (!m[i]) printf("allocation failure 2 in topobimat()\n");
      m[i] -= ncl;
      }

      return m;
       }
}
//2d integer matrix
int **ibimat(nrl,nrh,ncl,nch)
int nrl,nrh,ncl,nch;
{
   register int i,j;
   int **m;

   m=(int **) malloc((size_t) (nrh-nrl+1)*sizeof(int *));
   if (!m) {
                 printf("allocation failure 1 in ibimat()\n");
                 return(0);
                }
       else
         {
      m -= nrl;


      for(i=nrl;i<=nrh;i++) {
      m[i]=(int *) malloc((size_t) (nch-ncl+1)*sizeof(int));
      if (!m[i]) printf("allocation failure 2 in ibimat()\n");
      m[i] -= ncl;
      }

      return m;
       }
}
//allocates 2d character matrix
unsigned char **cbimat(nrl,nrh,ncl,nch)
int nrl,nrh,ncl,nch;
{
   register int i,j;
   unsigned char **m;

   m=(unsigned char **) malloc((size_t) (nrh-nrl+1)*sizeof(unsigned char *));
   if (!m) {
                 printf("allocation failure 1 in cbimat()\n");
                 return(0);
                }
       else
         {
      m -= nrl;


      for(i=nrl;i<=nrh;i++) {
      m[i]=(unsigned char *) malloc((size_t) (nch-ncl+1)*sizeof(unsigned char));
      if (!m[i]) printf("allocation failure 2 in cbimat()\n");
      m[i] -= ncl;
      }

      return m;
       }
}

//allocates 1d character matrix
unsigned char *cmat(nrl,nrh)
int nrl,nrh;
{
	unsigned char *m;

   m=(unsigned char *) malloc((size_t) (nrh-nrl+1)*sizeof(unsigned char));
   if (!m)
   {
       printf("allocation failure 1 in cmat()\n");
	   exit(0);
       return(0);
   }
   else
   {
      return m;
   }
}
//allochates 1d topology matrix
struct topology *topomat(nrl,nrh)
int nrl,nrh;
{
   struct topology *m;

   m=(struct topology *) malloc((size_t) (nrh-nrl+1)*sizeof(struct topology));
   if (!m)
   {
       printf("allocation failure 1 in topomat()\n");
	   exit(0);
       return(0);
   }
   else
   {
      return m;
   }
}
//allocates 1d double matrix
struct point *dmat(nrl,nrh)
int nrl,nrh;
{
   struct point *m;

   m=(struct point *) malloc((size_t) (nrh-nrl+1)*sizeof(struct point));
   if (!m)
   {
       printf("allocation failure 1 in dmat()\n");
	   exit(0);
       return(0);
   }
   else
   {
      return m;
   }
}
//allocates 1d integer matrix
int *imat(nrl,nrh)
int nrl,nrh;
{
   int *m;

   m=(int *) malloc((size_t) (nrh-nrl+1)*sizeof(int));
   if (!m)
   {
       printf("allocation failure 1 in imat()\n");
	   exit(0);
       return(0);
   }
   else
   {
      return m;
   }
}

//frees 3d matrix
void free_trimat(m,nrl,nrh,ncl,nch,nzl,nzh)
char ***m;
int nrl,nrh,ncl,nch,nzl,nzh;
{
   register int i, j;


   for(i=nrl;i<=nrh;i++)
   {
      for(j=ncl;j<=nch;j++)
      {
         free((char *) m[i][j]);
      }
      free ((char *) m[i]);
   }

   free ((char *) m);
}
//frees 2d matrix
void free_bimat(m,nrl,nrh,ncl,nch)
char **m;
int nrl,nrh,ncl,nch;
{
   register int i;

   for(i=nrh;i>=nrl;i--) free((struct topology *) (m[i]+ncl));
   free((char *) (m+nrl));
}
//frees 1d matix
void free_mat(m,nrl,nrh)
char **m;
int nrl,nrh;
{

   free((char *) (m));
}
