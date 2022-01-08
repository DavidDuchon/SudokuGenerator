#include <iostream>
#include <random>
#include <algorithm>
#include <array>
#include <string>
#include <bsoncxx/json.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/stdx.hpp>
#include <mongocxx/uri.hpp>
#include <mongocxx/instance.hpp>
#include <bsoncxx/builder/stream/helpers.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/builder/stream/array.hpp>

using bsoncxx::builder::stream::close_array;
using bsoncxx::builder::stream::close_document;
using bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::finalize;
using bsoncxx::builder::stream::open_array;
using bsoncxx::builder::stream::open_document;

bool validInSquare(int number, int x,int y, int (&sudoku)[9][9]){

	int column = x/3;
	int row = y/3;
	
	for(int i = 0;i < 3;i++){
		for(int j = 0;j < 3;j++){
			if ((column*3 + j != x) && (row*3 + i != y)){

				if (sudoku[row*3 + i][column*3 + j] == number){
					return false;
				}
			}
		}
	}
	return true;
}

bool validInRow(int number, int x, int y, int (&sudoku)[9][9]){

	for(int i = 0;i < 9;i++){
		if (i != x){
			if (sudoku[y][i] == number){
				return false;
			}
		}
	}

	return true;
}

bool validInColumn(int number, int x, int y, int (&sudoku)[9][9]){
	for(int i = 0;i < 9;i++){
		if (i != y){
			if (sudoku[i][x] == number){
				return  false;
			}
		}
	}

	return true;
}
		


bool validNumber(int number, int x , int y, int (&sudoku)[9][9]){
	if (!validInSquare(number, x, y, sudoku)){
		return false;
	}

	if (!validInRow(number,x,y,sudoku)){
		return false;
	}

	if (!validInColumn(number,x,y,sudoku)){
		return false;
	}

	return true;
}

bool validSudoku(int (&sudoku)[9][9]){
	for(int y = 0;y < 9;y++){
		for(int x = 0;x < 9;x++){
			if (!(validNumber(sudoku[y][x],x,y,sudoku) && (sudoku[y][x] != 0)))
				return false;
		}
	}
	
	return true;

}

int solveSudoku(int (&sudoku)[9][9],int count = 0){

	if (validSudoku(sudoku)){
		count++;
	}

	if(count > 1){
		return 2;
	}


	for(int y = 0;y < 9;y++){
		for(int x = 0;x < 9;x++){
			if (sudoku[y][x] == 0){
				for(int number = 1;number < 10;number++){

					if (validNumber(number,x,y,sudoku)){

						sudoku[y][x] = number;

						count = solveSudoku(sudoku,count);
						sudoku[y][x] = 0;

						if (count > 1){
							return 2;
						}
						
						
					}

				

				}
				return count;
				
				
			}
		}
	}

	return count;
}


bool generateSudoku(int (&sudoku)[9][9]){

	std::random_device rd;
	std::mt19937 generator(rd());
	std::array<int,9> numbers{1,2,3,4,5,6,7,8,9};

	for(int y = 0;y < 9;y++){

		for(int x = 0;x < 9;x++){

			if(sudoku[y][x] == 0){
					
				std::shuffle(numbers.begin(),numbers.end(),generator);

				for(int number:numbers){

					if(validNumber(number,x,y,sudoku)){
						
						sudoku[y][x] = number;
					
						if(generateSudoku(sudoku)){

							return true;

						}

					}

					sudoku[y][x] = 0;
				}

				return false;

			}

		}

	}

	return true;

}

void emptySudoku(int (&sudoku)[9][9]){
	for(int i = 0;i < 9;i++){
		for(int j = 0;j < 9;j++){
			sudoku[i][j] = 0;
		}
	}
}

void printSudoku(int (&sudoku)[9][9]){

	for(int i = 0;i < 9;i++){
		for(int j = 0;j < 9;j++){

			std::cout<<sudoku[i][j];
		}
		std::cout<<std::endl;
	}

	std::cout<<std::endl<<std::endl;
}


void generateValidSudoku(int (&sudoku)[9][9], int countToRemove){

	int number,x,y,countOfSolutions;

	std::array<int,9> numbersX {0,1,2,3,4,5,6,7,8};
	std::array<int,9> numbersY {0,1,2,3,4,5,6,7,8};

	std::random_device rd1;
	std::random_device rd2;
	std::mt19937 generator1(rd1());
	std::mt19937 generator2(rd2());

	for(int i = 0;i < countToRemove;i++){
		do{
			std::shuffle(numbersX.begin(),numbersX.end(),generator1);
			std::shuffle(numbersY.begin(),numbersY.end(),generator2);
		
			for(int xNum:numbersX){
				for(int yNum:numbersY){
					if (sudoku[yNum][xNum] != 0){
						x = xNum;
						y = yNum;
						break;
					}
				}
			}


			number = sudoku[y][x];

			sudoku[y][x] = 0;

			countOfSolutions = solveSudoku(sudoku);

			if(countOfSolutions != 1){
				sudoku[y][x] = number;
			}


		}while(countOfSolutions != 1);

	}

	printSudoku(sudoku);
		
}

int main(){

	mongocxx::instance instance{};
	mongocxx::uri uri("mongodb://localhost:27017");
	mongocxx::client client(uri);

	mongocxx::database db = client["sudoku"];

	int sudoku[9][9], d {0};

	std::array<std::string,3> difficulties {"easy","medium","hard"};

	for(const std::string& difficulty:difficulties){
		std::cout<<difficulty<<std::endl;

		mongocxx::collection collection = db[difficulty];
		
		for(int i = 0; i < 100 - 40*d;i++){

			auto dataBuilder = document{}; 
			
			emptySudoku(sudoku);

			generateSudoku(sudoku);

			dataBuilder << "_id" << i;

			auto arrayBuilder = dataBuilder << "solvedSudoku" << open_array;

			for(int i = 0;i < 9;i++){
				arrayBuilder << open_array;
				for(int j = 0;j < 9;j++){
					arrayBuilder << sudoku[i][j];
				}
				arrayBuilder << close_array;
			}

			arrayBuilder << close_array;

			generateValidSudoku(sudoku,40 +d*7);

			auto arrayBuilder2 = dataBuilder << "sudoku" << open_array;

			for(int i = 0;i < 9;i++){
				arrayBuilder2 << open_array;
				for(int j = 0;j < 9;j++){
					arrayBuilder2 << sudoku[i][j];
				}

				arrayBuilder2 << close_array;
			}

			arrayBuilder2 << close_array;

			bsoncxx::document::value doc_value = dataBuilder << bsoncxx::builder::stream::finalize;

			std::cout << bsoncxx::to_json(doc_value) << std::endl;

			collection.insert_one(doc_value.view());

		}

		d++;
	}

	return 0;
}
