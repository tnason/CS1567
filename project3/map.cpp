#include "map.h"
#include "logger.h"

Map::Map(RobotInterface *robotInterface, int startingX, int startingY) {
	_robotInterface = robotInterface;
	_score1 = 0;
	_score2 = 0;
	_loadMap();
	// we are the robot at this cell
	_setRobotAt(startingX, startingY);
	_curCell = cells[startingX][startingY];
	LOG.write(LOG_LOW, "map", "starting cell: %d, %d",
			  startingX, startingY);
}

Map::~Map() {
	for (int x = 0; x < MAP_WIDTH; x++) {
		for (int y = 0; y < MAP_HEIGHT; y++) {
			delete cells[x][y];
		}
	}
}

void Map::update() {
	map_obj_t *map = _robotInterface->getMap(&_score1, &_score2);

	// iterate through the linked list map
	// and update each cell
	while (map != NULL) {
		int x = map->x;
		int y = map->y;

		cells[x][y]->update(map);

		map = map->next;
	}
	setOpenings(0,0, 128);
}

int Map::getRobot1Score() {
	return _score1;
}

int Map::getRobot2Score() {
	return _score2;
}

Cell* Map::getCurrentCell() {
	return _curCell;
}

bool Map::occupyCell(int x, int y) {
	if (cells[x][y]->occupy(_robotInterface)) {
		_curCell = cells[x][y];
		return true;
	}
	return false;
}

bool Map::reserveCell(int x, int y) {
	return cells[x][y]->reserve(_robotInterface);
}

void Map::_setRobotAt(int x, int y) {

	cells[x][y]->setRobot();
}

void Map::_loadMap() {
	// load the map to start with and fill in our
	// cell matrix
	map_obj_t *map = _robotInterface->getMap(&_score1, &_score2);

	// iterate through the linked list map
	while (map != NULL) {
		int x = map->x;
		int y = map->y;

		cells[x][y] = new Cell(map);
		map = map->next;
	}

	setOpenings(0, 0, 128);
}

void Map::setOpenings(int x, int y, int cameFrom){
	LOG.write(LOG_LOW, "map_openings", "Now setting: x: %d, y: %d\n", x, y);
	cells[x][y]->addOpening(cameFrom);
	
	if(x+1 < MAP_WIDTH && cameFrom != 1){
		if(!cells[x+1][y]->isBlocked()){
			setOpenings(x+1,y,4);
			cells[x][y]->addOpening(1);
		}
		else{
			cells[x][y]->deleteOpening(1);
		}
	}
	if(x-1 > 0 && cameFrom != 4){
		if(!cells[x-1][y]->isBlocked()){
			setOpenings(x-1,y, 1);
			cells[x][y]->addOpening(4);
		}
		else{
			cells[x][y]->deleteOpening(4);
		}
	}
	if(y+1 < MAP_HEIGHT && cameFrom != 2){
		if(!cells[x][y+1]->isBlocked()){
			setOpenings(x,y+1, 8);
			cells[x][y]->addOpening(2);
		}
		else{
			cells[x][y]->deleteOpening(2);
		}
	}
	if(y-1 > 0 && cameFrom != 8){
		if(!cells[x][y-1]->isBlocked()){
			setOpenings(x,y-1, 2);
			cells[x][y]->addOpening(8);
		}
		else{
			cells[x][y]->deleteOpening(8);
		}
	}
}
