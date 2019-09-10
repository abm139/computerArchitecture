#include <stdio.h>
#include <stdlib.h>

void printM(double **matrix, int row, int col);
double **transpose(double **M, int row, int col);
double **multiplyMatrices(double** first, double** second, int r1, int c1, int r2, int c2);
double **inverse(double **M, int length);
void freeArr(double **M, int col);

int main(int argc, char *argv[]) {
    FILE *fptr;
    FILE *fptr1;
    fptr = fopen(argv[1], "r");
    fptr1 = fopen(argv[2], "r");
	
    int col, row, testrows;
    fscanf(fptr,"%d",&col);
    fscanf(fptr,"%d",&row);
    fscanf(fptr1,"%d",&testrows);
	col = col+1;

	double **X = malloc (sizeof (double) * row);
	double **Y = malloc(sizeof (double) * row);
	double **test = malloc(sizeof (double) * testrows);
	
	int i, j;
    for(i = 0; i<row; i++)
    {
    	*(X + i) = malloc (sizeof (double) * col);   
    	*(Y + i) = malloc (sizeof (double));
    }	
	
    for(i = 0; i<testrows; i++) 
    	*(test + i) = malloc (sizeof (double) * col);

	char *rowStr = malloc(sizeof(char) * 1500);
	char *value = malloc(sizeof(char) * 500);
	int rowIndex, valIndex;

   	for(i=0;i<row;i++)
	{
		fscanf(fptr,"%s", rowStr);
		rowIndex = 0;
		X[i][0] = 1;
    	for(j = 1; j < col + 1; j++)
		{
			for(valIndex = 0; rowStr[rowIndex] != ','; valIndex++)
			{
				if(rowStr[rowIndex] == '\0')
				{
					break;
				}
				value[valIndex] = rowStr[rowIndex];
				rowIndex++;
			}
			value[valIndex] = '\0';
			if(rowStr[rowIndex] == '\0')
			{
				Y[i][0] = atof(value);
			}
			else
			{
				X[i][j] = atof(value);
			}
			rowIndex++;		
		}
	}
	
   	for(i=0;i<testrows;i++)
	{
		fscanf(fptr1,"%s", rowStr);
		rowIndex = 0;
		test[i][0] = 1;
		
    	for(j = 1; j < col; j++)
		{
			for(valIndex = 0; rowStr[rowIndex] != ','; valIndex++)
			{
				if(rowStr[rowIndex] == '\0')
				{
					break;
				}
				value[valIndex] = rowStr[rowIndex];
				rowIndex++;
			}
			value[valIndex] = '\0';
			test[i][j] = atof(value);
			rowIndex++;
		}
		
	}
	double **xtrans = transpose(X, row, col);
	double **xtransX = multiplyMatrices(xtrans, X, col, row, row, col);
	double **invMat = inverse(xtransX, col);
	double **invTimesTransMat = multiplyMatrices(invMat, xtrans, col, col, col, row);
	double **W = multiplyMatrices(invTimesTransMat, Y, col, row, row, 1);
	double **final = multiplyMatrices(test, W, testrows, col, col, 1);
	printM(final, testrows, 1);
	free(rowStr);
	free(value);
	fclose(fptr);
	fclose(fptr1);
	freeArr(X, col);
	freeArr(Y, 1);
	freeArr(xtrans, col);
	freeArr(invMat, col);
	freeArr(xtransX, col);
	freeArr(invTimesTransMat, col);
	freeArr(W, 1);
	freeArr(final, 1);
	return 0;
}

void printM(double **M, int row, int col)
{
	int i, j;
	for(i=0;i<row;i++)
	{
       for(j=0;j<col;j++)
		{
           	printf("%0.0lf", M[i][j]);
		}
		printf("\n");
	}
}

double ** inverse(double **M, int length)
{
	int i, j, k;
	double result, c;
	double **inverse = malloc(sizeof(double) * length);
	for(i = 0; i<length; i++) 
	{
    	*(inverse + i) = malloc(sizeof (double) * length);
    }
    
    i = 0;
    j = 0; 
    while(i<length)
    {
      	while(j<length)
      	{
      		if(i == j)
      			inverse[i][j] = 1;
      		else
      			inverse[i][j] = 0;
      		j++;
      	}
      	j=0;
      	i++;
    }
    
    i = 0;
    while(i < length)
	{
		j = 0;
		while(j< length)
		{
			if(j > i)
			{
				j++;
				continue;
			}
			else if(j == i)
			{
				k = 0;
				c =  M[i][j];
				for(k = 0; k < length; k++)
				{
					result = inverse[j][k] / c;
					M[j][k] = M[j][k] / c;
					inverse[j][k] = result;
				}
				
			}
			else
			{
				k = 0;
				c = M[i][j];
				for(k = 0; k < length; k++)
				{
					result = inverse[i][k] - (inverse[j][k] * c);
					M[i][k] -= M[j][k] * c;
					inverse[i][k] = result;
				}
			}
			j++;
		}
		i++;
	}
	
	for(i = length - 2; i >= 0 ; i--)
		for(j = length - 1; j > i; j--)
		{
			k = 0;
			c = M[i][j];
			for(k = 0; k < length; k++)
			{
				result = inverse[i][k] - (inverse[j][k] * c);
				M[i][k] -= M[j][k] * c;
				inverse[i][k] = result;
			}
		}
	
	
    return inverse;
}

double ** transpose(double **M, int row, int col)
{
	int i;
	int j;
	double **transpose = malloc(sizeof(double) * col);
	for(i = 0; i<col; i++) 
	{
    	*(transpose + i) = malloc(sizeof (double) * row);
    }
    i = 0;
    j = 0; 
    while(i<row)
    {
      	while(j<col)
      	{
      		transpose[j][i] = M[i][j];
      		j++;
      	}
      	j=0;
      	i++;
    }
    return transpose;
}

double **multiplyMatrices(double** first, double** second, int r1, int c1, int r2, int c2)
{
    int i, j, k;
    double ** result;
    result = malloc(r1 * sizeof(double));
    for(i = 0; i < r1; i++) 
    	*(result + i) = malloc(sizeof(double) * c2);
	
	for (i = 0; i < r1; i++)
        for (j = 0; j < c2; j++)
			result[i][j] = 0;
	
    for (i = 0; i < r1; i++)
        for (j = 0; j < c2; j++)
            for (k = 0; k < c1; k++)
               result[i][j] += first[i][k] * second[k][j];
	return result;
}

void freeArr(double **M, int col)
{
	int i;
	i = 0;
	for(i = 0; i < col; i++)
    	free(M[i]);
	free(M);
}
