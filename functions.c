#include "structs.h"

// Checks the arguments that were provided in the command line
int check_args(int argc, char **argv, int *buckets, int *N){

	if(argc != 2){
		printf("Error! Invalid arguments\n");
		return -1;
	}

	*N = atoi(argv[1]);
	*buckets = pow(2,*N);

	return 0;
}

// Allocates memory for the histogram array of a table and initializes it
int allocate_hist(int **hist_ptr, int buckets){

	// Allocating memory for the histogram array of the table
	*hist_ptr = (int *)malloc(buckets*sizeof(int));
	if(*hist_ptr == NULL){
		perror("Memory allocation failed: ");
		return -1;
	}

	// Filling in the histogram array with zeros
	for(int i = 0; i < buckets; i++)
		hist_ptr[0][i] = 0;
	
	return 0;
}

// Allocates memory for the psum array of a table and initializes it
int allocate_psum(int **psum_ptr, int buckets){

	*psum_ptr = (int *)malloc(buckets*sizeof(int));
	if(*psum_ptr == NULL){
		perror("Memory allocation failed: ");
		return -1;
	}

	for(int i = 0; i < buckets; i++)
		psum_ptr[0][i] = -1;

	return 0;
}

int allocate_histograms(int ***hist, int ***psum, int tables, int buckets){

	/* Allocating memory for a table of pointers. Each pointer points to the
	   hist array of the corresponding table */ 
	*hist = (int **)malloc(tables*sizeof(int *));
	if(*hist == NULL){
		perror("Memory allocation failed: ");
		return -1;
	}

	*psum = (int **)malloc(tables*sizeof(int *));
	if(*psum == NULL){
		perror("Memory allocation failed: ");
		return -1;
	}

	// Working for one table at a time
	for(int i = 0; i < tables; i++){
		if((allocate_hist(&hist[0][i],buckets) < 0) || 
			allocate_psum(&psum[0][i],buckets) < 0)
			return -1;
	}

	return 0;
}
	
// Creates a table which has random values
int allocate_a_table(struct relation **table_ptr, int records){

	*table_ptr = (struct relation *)malloc(sizeof(struct relation));
	if(*table_ptr == NULL){
		perror("Memory allocation failed: ");
		return -1;
	}

	table_ptr[0]->num_tuples = ((rand()%10/*00*/)+1);/*records*/;
	table_ptr[0]->tuples = (struct tuple *)malloc(table_ptr[0]->num_tuples*
		sizeof(struct tuple));

	if(table_ptr[0]->tuples == NULL){
		perror("Memory allocation failed: ");
		return -1;
	}

	for(int i = 0; i < table_ptr[0]->num_tuples; i++){
		table_ptr[0]->tuples[i].key = rand() % records;
		table_ptr[0]->tuples[i].payload = 0;
	}

	return 0;
}

int allocate_tables(struct relation ***table, int records, int tables){

	// Initially, we're filling in the tables with random integers
	long curtime = time(NULL);
	srand((unsigned int)curtime);

	// Creating an array of pointers. Each one of them points to a table
	*table = (struct relation **)malloc(tables*sizeof(struct relation *));
	if(*table == NULL){
		perror("Memory allocation failed: ");
		return -1;
	}

	// Creating a table
	for(int i = 0; i < tables; i++){
		if(allocate_a_table(&table[0][i],records) < 0)
			return -1;
	}

	return 0;
}

// Working with one table at a time
int allocate_a_final_table(struct relation **final_table_ptr, int num_tuples){
	
	*final_table_ptr = (struct relation *)malloc(sizeof(struct relation));
	if(*final_table_ptr == NULL){
		perror("Memory allocation failed: ");
		return -1;
	}

	final_table_ptr[0]->num_tuples = num_tuples;
	final_table_ptr[0]->tuples = (struct tuple *)malloc(sizeof(struct tuple)*
		final_table_ptr[0]->num_tuples);

	if(final_table_ptr[0]->tuples == NULL){
		perror("Memory allocation failed: ");
		return -1;
	}

	for(int i = 0; i < final_table_ptr[0]->num_tuples; i++){
		final_table_ptr[0]->tuples[i].key = -1;
		final_table_ptr[0]->tuples[i].payload = -1;
	}

	return 0;
}

// Allocates memory for the final tables
int allocate_final_tables(struct relation ***final_table, 
	struct relation **input_table, int total_tables){

	*final_table = (struct relation **)malloc(total_tables*
		sizeof(struct relation *));
	if(*final_table == NULL){
		perror("Memory allocation failed: ");
		return -1;
	}

	for(int i = 0; i < total_tables; i++){
		if(allocate_a_final_table(&final_table[0][i],
			input_table[i]->num_tuples) < 0)
			return -1;
	}

	return 0;
}

// Fills in the hist array of a table with the correct values
void fill_hist(struct relation *table, int *hist, int buckets){

	for(int i = 0; i < table->num_tuples; i++){
		int hash_value = table->tuples[i].key % buckets;
		hist[hash_value]++;
	}
}

// Fills in the psum array of a table with the correct values
void fill_psum(int *hist, int *psum, int buckets){
	
	int last = 0;

	for(int i = 0; i < buckets; i++){

		// If the bucket has at least one element
		if(hist[i] != 0){
			psum[i] = last;
			last += hist[i];
		}
	}
}

// Fills in both of the histogram arrays with the correct values
void fill_histograms(struct relation **input_tables, int **hist, int **psum,
	int buckets, int total_tables){

	for(int i = 0; i < total_tables; i++){
		fill_hist(input_tables[i],hist[i],buckets);
		fill_psum(hist[i],psum[i],buckets);
	}
}

/*******************************/
/*** Rearrangement Functions ***/
/*******************************/

void rearrange_a_table(struct relation *final_table,
	struct relation *input_table, int *psum, int buckets){

	// Iterate the values one by one
	for(int i = 0; i < final_table->num_tuples; i++){

		// Find the index of the bucket whom the value belongs to
		int hash_value = input_table->tuples[i].key % buckets;

		// Find out where the bucket begins
		int index = psum[hash_value];

		// Iterate the final table until you find an empty spot
		while(final_table->tuples[index].key != -1){
			index++;
		}

		final_table->tuples[index].key = input_table->tuples[i].key;
	}	
}

void rearrange_tables(int total_tables, struct relation **input_tables,
	struct relation **final_tables, int **psum, int buckets){

	for(int i = 0; i < total_tables; i++){
		rearrange_a_table(final_tables[i],input_tables[i],psum[i],buckets);
	}
}

/*********************************/
/*** Functions for the Indeces ***/
/*********************************/

/* Compares all of the buckets which have the same index, in order to find the 
   one that has the least amount of data. Then it returns the value
   that represents the least amount of data along with the index of the table 
   it belongs to */
void get_min_data(int total_tables, int bucket_index, int **hist, 
	int *table_index, int *total_data){

	int min_value;
	int min_table;
	int has_min_value = 0;

	// Traverse each one the hist histograms
	for(int i = 0; i < total_tables; i++){

		if(!has_min_value && hist[i][bucket_index] != 0) {
			has_min_value = 1;
			min_value = hist[i][bucket_index];
			min_table = i;
		}
		else if(has_min_value && hist[i][bucket_index] < min_value &&
			hist[i][bucket_index] != 0){
			min_value = hist[i][bucket_index];
			min_table = i;
		}
	}

	/* All of the buckets with index = bucket_index don't have
	   any data in them */
	if(!has_min_value){
		min_value = -1;
		min_table = -1;
	}

	*total_data = min_value;
	*table_index = min_table;
}

// Checks whether the provided number is prime or not
int is_prime(int num){

     if(num <= 1)
     	return 0;
     if(num % 2 == 0 && num > 2)
     	return 0;

     for(int i = 3; i < num / 2; i+= 2){
     	if (num % i == 0)
        	return 0;
     }

     return 1;
}

// Allocating memory for the chain array of a bucket
int allocate_chain(struct index_array *ptr){

	ptr->chain = (int *)malloc((ptr->total_data+1)*sizeof(int));
	if(ptr->chain == NULL){
		perror("Memory allocation failed: ");
		return -1;
	}

	for(int i = 0; i < ptr->total_data+1; i++)
		ptr->chain[i] = -1;

	return 0;
}

int allocate_bucket(struct index_array *ptr){

	int j = ptr->total_data;
	while(is_prime(j) == 0)
		j++;
	ptr->bucket_size = j;

	ptr->bucket = (int *)malloc((ptr->bucket_size)*sizeof(int));
	if(ptr->bucket == NULL){
		perror("Memory allocation failed: ");
		return -1;
	}

	for(int i = 0; i < ptr->bucket_size; i++)
		ptr->bucket[i] = -1;

	return 0;
}

int fill_an_index(struct index_array *ptr, int bucket_index, int total_tables,
	int **hist){

	get_min_data(total_tables,bucket_index,hist,&(ptr->table_index),
		&(ptr->total_data));

	// The bucket whose index is (bucket_index) doesn't have any data
	if(ptr->table_index == -1 && ptr->total_data == -1){
		ptr->bucket_size = -1;
		ptr->chain = NULL;
		ptr->bucket = NULL;
	}
	else{
		if((allocate_chain(ptr) < 0) || (allocate_bucket(ptr) < 0))
			return -1;
	}

	return 0;
}

int allocate_index_array(struct index_array **my_array, int buckets, 
	int total_tables, int **hist){

	*my_array = (struct index_array *)malloc(buckets*sizeof(struct index_array));
	if(*my_array == NULL){
		perror("Memory allocation failed: ");
		return -1;
	}

	for(int i = 0; i < buckets; i++){
		if(fill_an_index(&my_array[0][i],i,total_tables,hist) < 0)
			return -1;
	}

	return 0;
}

void fill_indeces(struct index_array *my_array, int buckets, int **psum, 
	struct relation **final_table){
		
	for(int i = 0; i < buckets; i++){

		// The bucket whose index is (i) doesn't have any data. Move on
		if(my_array[i].table_index == -1)
			continue;

		// The table that the bucket belongs to
		int current_table = my_array[i].table_index;

		// Where the bucket begins in the reordered table
		int start_index = psum[current_table][i];

		// Where the bucket ends in the reordered table
		int end_index = start_index + my_array[i].total_data -1;

		for(int j = end_index; j > start_index -1; j--){

			int hash_value = final_table[current_table]->tuples[j].key % my_array[i].bucket_size;

			/* IMPORTANT NOTE: We have to think of each bucket as an autonomous
			   array, i.e starting from 0 and ending at total_data - 1. 
			   (Match: actual_index - start_index) */

			// This is the first time we've come across such hash value
			if(my_array[i].bucket[hash_value] == -1){
				my_array[i].bucket[hash_value] = ((j+1) - start_index);
				my_array[i].chain[((j+1) - start_index)] = 0;
			}	
			else{

				int tmp = my_array[i].bucket[hash_value];
				while(my_array[i].chain[tmp] != 0){
					tmp = my_array[i].chain[tmp];
				}
				
				my_array[i].chain[tmp] = ((j+1) - start_index);
				my_array[i].chain[((j+1) - start_index)] = 0;
			}
		}
	}
}

/**************************/
/*** Printing Functions ***/
/**************************/

// Prints the histogram array of every table
void print_hist(int **hist, int tables, int buckets){

	for(int i = 0; i < tables; i++){
		for(int j = 0; j < buckets; j++)
			printf("hist[%d][%d]: %d\n",i,j,hist[i][j]);
		
		printf("\n");
	}
}	

// Prints the psum array of every table
void print_psum(int **psum, int tables, int buckets){

	for(int i = 0; i < tables; i++){
		for(int j = 0; j < buckets; j++)
			printf("psum[%d][%d]: %d\n",i,j,psum[i][j]);
		
		printf("\n");
	}
}

// Prints all of the records of every table that's available
void print_tables(struct relation **tables, int total_tables){

	for(int i = 0; i < total_tables; i++){
		for(int j = 0; j < tables[i]->num_tuples; j++){
			printf("table[%d][%d]: %d\n",i,j,tables[i]->tuples[j].key);
		}
		printf("\n");
	}
}

void print_index_array(int buckets, struct index_array *my_array){

	for(int i = 0; i < buckets; i++){
		printf("my_array[%d].table_index: %d\n",i,my_array[i].table_index);
		printf("my_array[%d].total_data: %d\n",i,my_array[i].total_data);
		printf("my_array[%d].bucket_size: %d\n",i,my_array[i].bucket_size);

		if(my_array[i].table_index != -1){
			printf("\n");
			for(int j = 0; j < (my_array[i].total_data)+1; j++)
				printf("my_array[%d].chain[%d]: %d\n",i,j,my_array[i].chain[j]);
		
			printf("\n");
			for(int j = 0; j < my_array[i].bucket_size; j++)
				printf("my_array[%d].bucket[%d]: %d\n",i,j,my_array[i].bucket[j]);
		}
		printf("\n");
	}
}

// Prints the total number of records each table has
void print_records_no(int tables, struct relation **table){
	for(int i = 0; i < tables; i++)
		printf("Table %d has %d records\n",i,table[i]->num_tuples);
	printf("\n");
}

// Displays a message with all the possible options a user has
void print_welcome_msg(int beginning){
	
	if(beginning)
		printf("Welcome!\n");
	printf("Please, choose one of the following options:\n");
	printf("* Query: To retrieve data.\n");
	printf("* Exit: To exit the application.\n");
}

/***********************************/
/*** Communication with the user ***/
/***********************************/

// Responsible for getting and interpreting the input that the user provides
int get_user_input(char **input, size_t *n){

	print_welcome_msg(1);

	/* Keep getting input from the user, until he/she decides to exit 
	   the program */
	while(getline(input,n,stdin) != -1){

		// The user wants to retrieve certain data
		if(strncmp(*input,"Query",strlen("Query")) == 0){

			printf("Enter the query/-ies file name: ");
			char *queriesFileName = NULL;
			size_t qsize = 0;
			FILE *fin = NULL;
			if(getline(&queriesFileName,&qsize,stdin) != -1){
				printf("File to be executed:%s\n", queriesFileName);
				fin = fopen(strtok(queriesFileName,"\n"),"r");
				if (fin == NULL)
				{
					perror("Queries file opening failed:");
					return -1;
				}
			}

			/* Variables for the following getline() function.
			   They are initialized with NULL and 0 respectively, 
			   so that getline() can allocate the appropriate ammount 
			   of memory. */
			char *query = NULL;	// The actual query the user gives
			size_t n1 = 0;		
			
			/* We'll keep accepting queries from the user until he/she types
			   the letter 'F'. 
			   Every query consists of three parts and has the following form: 

			   0 2 4|0.1=1.2&1.0=2.1&0.1>3000|0.0 1.1 */

			int numQueries = 0;

			while(getline(&query,&n1,fin) != -1){

				// The user wants to stop providing us with queries
				if(strncmp(query,"F",strlen("F")) == 0){
					printf("We're done with accepting queries.\n");
					break;
				}

				else{

					/* We must split the given query into its three parts.
					   Since we'll be using the strtok() function, we must
					   make a copy of the provided query */
					char *query_copy;
					query_copy = (char *)malloc(strlen(query)*sizeof(char));
					if(query_copy == NULL){
						perror("Memory allocation failed: ");
						return -1;
					}
					memset((char *)query_copy,0,strlen(query));
					strncpy(query_copy,query,strlen(query)-1);

					/* We'll store each part individually.
					   e.g: parts -> parts[0] -> "0 2 4"
					                 parts[1] -> "0.1=1.2&1.0=2.1&0.1>3000"
					                 parts[2] -> "0.0 1.1" */  
					char **parts;
					parts = (char **)malloc(3*sizeof(char *));
					if(parts == NULL){
						perror("Memory allocation failed: ");
						return -1;
					}

					/* Finding the three parts of the query, using strtok()
					   and the "|" character as the delimiter */
					for(int i = 0; i < 3; i++){

						if(i == 0){
							parts[i] = strtok(query_copy,"|"); 
						}
						else
							parts[i] = strtok(NULL,"|");
					}

					/*****************************************/
					/*** Storing the elements of each part ***/
					/*****************************************/

					/* (*) Each one of the following can (and should) be put 
					       in an individual function */
					
					char *tmp; // Temporary pointer used with strtok()
					
					/* We'll store the IDs of the tables that participate
                       in the JOIN statement as integers.
                                             -----
                       e.g: table_indeces -> | 0 |
                       						 -----
                       						 | 2 |
                       						 -----
                       						 | 4 |
                       						 -----                            */	
					int *table_indeces; // The array where we'll store the IDs
					int size = 0;  // The size of the table_indeces array

					// Getting the IDs of the tables
					while(1){

						// Getting the ID of the first table
						if(!size){
							tmp = strtok(parts[0]," ");

							if(tmp == NULL)
								break;

							// i.e size = 1
							size++;

							table_indeces = (int *)malloc(size*sizeof(int));
							if(table_indeces == NULL){
								perror("Memory allocation failed: ");
								return -1;
							}
							table_indeces[size-1] = atoi(tmp);
						}

						// Getting the ID of the remaining tables
						else{

							tmp = strtok(NULL," ");

							if(tmp == NULL)
								break;

							// i.e size = 1, 2, ...
							size++;
							
							table_indeces = (int *)realloc((int *)table_indeces,
								size*sizeof(int));

							if(table_indeces == NULL){
								perror("Memory reallocation failed: ");
								return -1;
							}

							table_indeces[size-1] = atoi(tmp);
						}
					}					


					/* We'll store each one of the filters as an individual 
					   string.
					   
					   e.g filters -> filters[0] -> "0.1=1.2"
					                  filters[1] -> "1.0=2.1"
					                  filters[2] -> "0.1>3000"                */
					char **filters;
					int size_1 = 0;
						
					while(1){

						if(!size_1){

							tmp = strtok(parts[1],"&");

							if(tmp == NULL)
								break;

							// i.e size_1 = 1
							size_1++;

							filters = (char **)malloc(size_1*sizeof(char *));
							if(filters == NULL){
								perror("Memory allocation failed: ");
								return -1;
							}

							filters[size_1-1] = (char *)malloc((strlen(tmp)+1)*
								sizeof(char));
							if(filters[size_1-1] == NULL){
								perror("Memory allocation failed: ");
								return -1;
							}

							strcpy(filters[size_1-1],tmp);
						}

						else{

							tmp = strtok(NULL,"&");

							if(tmp == NULL)
								break;

							// i.e size_1 = 2, 3, ...
							size_1++;

							filters = (char **)realloc((char **)filters,
								size_1*sizeof(char *));
							if(filters == NULL){
								printf("Memory reallocation failed: \n");
								return -1;
							}

							filters[size_1-1] = (char *)malloc((strlen(tmp)+1)*
								sizeof(char));
							if(filters[size_1-1] == NULL){
								perror("Memory allocation failed: ");
								return -1;
							}

							strcpy(filters[size_1-1],tmp);
						}
					}	

					/* In order to store the ID of the table along with one of 
					   its columns as integers, we have to split the original 
					   string first. 
					   
					   e.g: part3 -> part3[0] -> "0.0"
					                 part3[1] -> "1.1"                        */
					char **part3;
					int size_2 = 0;

					while(1){

						if(!size_2){

							tmp = strtok(parts[2]," ");
							if(tmp == NULL)
								break;

							// i.e: size_2 = 1
							size_2++;

							part3 = (char **)malloc(size_2*sizeof(char *));
							if(part3 == NULL){
								perror("Memory allocation failed: ");
								return -1;
							}

							part3[size_2-1] = (char *)malloc((strlen(tmp)+1)*
								sizeof(char));
							if(part3[size_2-1] == NULL){
								perror("Memory allocation failed: ");
								return -1;
							}
							memset((char *)part3[size_2-1],0,strlen(tmp)+1);
							strcpy(part3[size_2-1],tmp);
						}

						else{

							tmp = strtok(NULL," ");
							if(tmp == NULL)
								break;

							// i.e: size_2 = 2, 3, ...
							size_2++;

							part3 = (char **)realloc((char **)part3,size_2*
								sizeof(char *));
							if(part3 == NULL){
								perror("Memory reallocation failed: ");
								return -1;
							}

							part3[size_2-1] = (char *)malloc((strlen(tmp)+1)*
								sizeof(char));
							if(part3[size_2-1] == NULL){
								perror("Memory allocation failed: ");
								return -1;
							}
							memset((char *)part3[size_2-1],0,strlen(tmp)+1);
							strcpy(part3[size_2-1],tmp);
						}
					}


					/* Now we can store both the table and its column as
					   integers.             
					   						   table  column
											 -----------------	
					   e.g: sum -> sum[0] -> |   0   |   0   |
					   						 -----------------
					   			   sum[1] -> |   1   |   1   |
					   			   			 -----------------                */
					int **sum;
					sum = (int **)malloc(size_2*sizeof(int *));
					if(sum == NULL){
						perror("Memory allocation failed: ");
						return -1;
					}

					for(int i = 0; i < size_2; i++){

						sum[i] = (int *)malloc(2*sizeof(int));
						if(sum[i] == NULL){
							perror("Memory allocation failed: ");
							return -1;
						}

						tmp = strtok(part3[i],".");
						sum[i][0] = atoi(tmp);

						tmp = strtok(NULL,".");
						sum[i][1] = atoi(tmp);
					}

					/* Printing the results to ensure that everything 
					   works fine */ 
					// printf("You've requested the following:\n");
					// for(int i = 0; i < size; i++)
					// 	printf("table[%d]: %d, ",i,table_indeces[i]);
					
					// printf("\n");
					// for(int i = 0; i < size_1; i++)
					// 	printf("filters[%d]: %s, ",i,filters[i]);

					// printf("\n");
					// for(int i = 0; i < size_2; i++){
					// 	printf("sum[%d][0]: %d, ",i,sum[i][0]);
					// 	printf("sum[%d][1]: %d\n",i,sum[i][1]);
					// }

					// Freeing every piece of memory that we allocated.
					for(int i = 0; i < size_1; i++)
						free(filters[i]);
					
					for(int i = 0; i < size_2; i++)	{
						free(part3[i]);
						free(sum[i]);
					}

					free(sum);
					free(part3);
					free(filters);
					free(table_indeces);
					free(parts);
					free(query_copy);
					numQueries++;
				}
			}
			printf("Executed %d queries.\n", numQueries);
			fclose(fin);
			free(query);
			free(queriesFileName);
			print_welcome_msg(0);
		}

		// The user wants to exit the program
		else if(strncmp(*input,"Exit",strlen("Exit")) == 0){
			printf("Bye!\n");
			break;
		}

		else{
			printf("Wrong input. Try again.\n");
			print_welcome_msg(0);
		}
	}

	return 0;
}

/*************************************/
/*** Functions that release memory ***/
/*************************************/

void free_histograms(int ***hist, int ***psum, int total_tables){
	for(int i = 0; i < total_tables; i++){
		free(hist[0][i]);
		free(psum[0][i]);
	}

	free(*hist);
	free(*psum);
}

void free_tables(struct relation ***tables, struct relation ***final_tables, 
	int total_tables){
	for(int i = 0; i < total_tables; i++){
		free(tables[0][i]->tuples);
		free(tables[0][i]);

		free(final_tables[0][i]->tuples);
		free(final_tables[0][i]);
	}

	free(*tables);
	free(*final_tables);
}

void free_indeces(struct index_array **my_array, int buckets){

	for(int i = 0; i < buckets; i++){
		if(my_array[0][i].table_index != -1){
			free(my_array[0][i].chain);
			free(my_array[0][i].bucket);
		}
	}

	free(*my_array);
}

// Frees every piece of memory we previously allocated
void free_memory(int ***hist, int ***psum, int total_tables, 
	struct relation ***tables, struct relation ***final_tables,
	struct index_array **my_array, int buckets, char **input){

	free_histograms(hist,psum,total_tables);
	free_tables(tables,final_tables,total_tables);
	free_indeces(my_array,buckets);
	free(*input);
}