#include "map.h"
#include <stdlib.h>
#include <stdio.h>
#include <ncurses.h>

#define random(min, max) (rand() % (max-min+1) + min)

Coord coordBuilder(int x, int y) {
    Coord coord;
    coord.x = x;
    coord.y = y;
    return coord;
}

int isSpawnpoint(Box* box) {
    return box != NULL && box->playerId == -1 && box->fishes == 1 && box->fishValues[0] == 1;
}

Map* mapBuilder(int penguins, int width, int length) {
    Map* map = malloc(sizeof(Map));
    if (map == NULL) exit(1);

    map->length = length;
    map->width = width;


    int mapWinHeight = 5 + (map->width - 1) * 3;
    int mapWinLength = map->length * 8 + 1;

    map->mapWin = newwin(mapWinHeight, mapWinLength, 0, 0);
    if (map->mapWin == NULL) exit(1);

    int nBoxes = length*width - (width - width / 2); // remove number of odd lines
    map->nBoxes = nBoxes;

    map->boxes = malloc(nBoxes*sizeof(Box));
    if (map->boxes == NULL) exit(1);

    int spawnPoints = 0;
    for (int y = 0; y < width; ++y) {
        for (int x = 0; x < length; ++x) {
            Coord coord = coordBuilder(x, y);
            Box* box = getBox(map, coord, 1);
            if (box == NULL) continue;
            box->isMelt = 0;
            box->coord = coord;
            box->playerId = -1;
            box->fishes = random(1, 3);
            box->fishValues = malloc(sizeof(int)*box->fishes);
            if (box->fishValues == NULL) exit(1);
            for (int j = 0; j < box->fishes; j++) {
                box->fishValues[j] = random(1, 3);
            }
            if (isSpawnpoint(box)) spawnPoints++;
        }
    }

    while (spawnPoints < penguins) {
        Box* randomBox;
        do {
            randomBox = map->boxes + random(0, nBoxes-1);
        } while (isSpawnpoint(randomBox));
        randomBox->fishes = 1;
        randomBox->fishValues = realloc(randomBox->fishValues, sizeof(int));
        if (randomBox->fishValues == NULL) exit(1);
        randomBox->fishValues[0] = 1;
        spawnPoints++;
    }
    
    return map;
}

int isValidCoord(Map map, Coord coord) {
    if (coord.y >= map.width || coord.y < 0 || coord.x < 0) return 0;
    if (coord.y % 2 == 0) return coord.x < map.length - 1;
    return coord.x < map.length;
}

Box* getBox(Map* map, Coord coord, int ignoreMeltedBoxes) {
    if (!isValidCoord(*map, coord)) return NULL;
    int i = coord.x;
    i += (coord.y / 2) * map->length;
    i += (coord.y - coord.y/2) * (map->length-1);
    Box* box = map->boxes + i;
    if (box == NULL) return NULL;
    if (ignoreMeltedBoxes || !box->isMelt) return box;
    return NULL;
}

Box* getBoxFromId(Map* map, int boxId) {
    if (boxId < 0 || boxId >= map->nBoxes) return NULL;
    return map->boxes + boxId;
}

Box* getRelativeBox(Map* map, Coord coord, Direction direction) {
    int isColumnEven = coord.y % 2 == 0;
    switch (direction) {
        case WEST:
            coord.x--;
            break;
        case EAST:
            coord.x++;
            break;
        case NORTHWEST:
            if (!isColumnEven) coord.x--;
            coord.y--;
            break;
        case NORTHEAST:
            if (isColumnEven) coord.x++;
            coord.y--;
            break;
        case SOUTHWEST:
            if (!isColumnEven) coord.x--;
            coord.y++;
            break;
        case SOUTHEAST:
            if (isColumnEven) coord.x++;
            coord.y++;
            break;
        default:
            return NULL;
    }
    return getBox(map, coord, 1);
}

int getAvailableSteps(Map* map, Coord coord, Direction direction) {
    int steps = 0;
    Box* relative = getRelativeBox(map, coord, direction);
    while (relative != NULL && !relative->isMelt && relative->playerId == -1) {
        steps++;
        relative = getRelativeBox(map, relative->coord, direction);
    }
    return steps;
}

Box* getDistancedRelativeBox(Map* map, Coord coord, Direction direction, int distance) {
    if (distance < 0) return NULL;
    if (distance == 0) return getBox(map, coord, 1);
    Box* relative = getRelativeBox(map, coord, direction);
    if (relative == NULL || relative->isMelt || relative->playerId != -1) return NULL;
    return getDistancedRelativeBox(map, relative->coord, direction, distance-1);
}

void highlightBox(Box* box, WINDOW* mapWin, int color) {
    if (box == NULL) return;
    int x = box->coord.x, y = box->coord.y;
    int yOffset = y*3;
    int xOffset = x*8 + (y%2==0)*4;

    mvwchgat(mapWin, yOffset, xOffset + 4, 1, A_NORMAL, color, NULL);
    mvwchgat(mapWin, yOffset + 1, xOffset + 2, 5, A_NORMAL, color, NULL);
    mvwchgat(mapWin, yOffset + 2, xOffset + 1, 7, A_NORMAL, color, NULL);
    mvwchgat(mapWin, yOffset + 3, xOffset + 2, 5, A_NORMAL, color, NULL);
    mvwchgat(mapWin, yOffset + 4, xOffset + 4, 1, A_NORMAL, color, NULL);

    wrefresh(mapWin);
}

void removeHighlightBox(Box* box, WINDOW* mapWin) {
    highlightBox(box, mapWin, 13);
}

void meltBox(Box* box, Map* map) {
    if (box == NULL) return;
    box->isMelt = 1;
    box->playerId = -1;
    box->fishes = 0;
    free(box->fishValues);
    int x = box->coord.x, y = box->coord.y;
    x = x*8 + (y%2==0)*4;
    y = y*3;

    mvwprintw(map->mapWin, y + 1, x + 2, "     ");
    mvwprintw(map->mapWin, y + 2, x + 1, "       ");
    mvwprintw(map->mapWin, y + 3, x + 2, "     ");
    highlightBox(box, map->mapWin, 14);
    Box* northEast = getRelativeBox(map, box->coord, NORTHEAST);
    if (northEast == NULL || northEast->isMelt) {
        mvwchgat(map->mapWin, y, x + 5, 1, A_NORMAL, 14, NULL);
        mvwchgat(map->mapWin, y + 1, x + 7, 1, A_NORMAL, 14, NULL);
    }
    Box* southEast = getRelativeBox(map, box->coord, SOUTHEAST);
    if (southEast == NULL || southEast->isMelt) {
        mvwchgat(map->mapWin, y + 4, x + 5, 1, A_NORMAL, 14, NULL);
        mvwchgat(map->mapWin, y + 3, x + 7, 1, A_NORMAL, 14, NULL);
    }
    Box* southWest = getRelativeBox(map, box->coord, SOUTHWEST);
    if (southWest == NULL || southWest->isMelt) {
        mvwchgat(map->mapWin, y + 4, x + 3, 1, A_NORMAL, 14, NULL);
        mvwchgat(map->mapWin, y + 3, x + 1, 1, A_NORMAL, 14, NULL);
    }
    Box* northWest = getRelativeBox(map, box->coord, NORTHWEST);
    if (northWest == NULL || northWest->isMelt) {
        mvwchgat(map->mapWin, y, x + 3, 1, A_NORMAL, 14, NULL);
        mvwchgat(map->mapWin, y + 1, x + 1, 1, A_NORMAL, 14, NULL);
    }
    Box* east = getRelativeBox(map, box->coord, EAST);
    if (east == NULL || east->isMelt) {
        mvwchgat(map->mapWin, y + 2, x + 8, 1, A_NORMAL, 14, NULL);
    }
    Box* west = getRelativeBox(map, box->coord, WEST);
    if (west == NULL || west->isMelt) {
        mvwchgat(map->mapWin, y + 2, x, 1, A_NORMAL, 14, NULL);
    }

}

void printBox(Box* box, WINDOW* mapWin, int printBorder, int printFishes) {
    // TODO: higlight if player on it
    if (box == NULL) return;
    int x = box->coord.x, y = box->coord.y;
    int yOffset = y*3;
    int xOffset = x*8 + (y%2==0)*4;
    if (printBorder) {
        wattron(mapWin, COLOR_PAIR(13));
        mvwaddch(mapWin, yOffset, xOffset + 3, '/');
        mvwaddch(mapWin, yOffset, xOffset + 5, '\\');
        mvwaddch(mapWin, yOffset + 1, xOffset + 1, '/');
        mvwaddch(mapWin, yOffset + 1, xOffset + 7, '\\');
        mvwaddch(mapWin, yOffset + 2, xOffset, '|');
        mvwaddch(mapWin, yOffset + 2, xOffset + 8, '|');
        mvwaddch(mapWin, yOffset + 3, xOffset + 3, ' ');
        mvwaddch(mapWin, yOffset + 3, xOffset + 1, '\\');
        mvwaddch(mapWin, yOffset + 3, xOffset + 7, '/');
        mvwaddch(mapWin, yOffset + 4, xOffset + 3, '\\');
        mvwaddch(mapWin, yOffset + 4, xOffset + 5, '/');
        wattroff(mapWin, COLOR_PAIR(13));
    }

    if (printFishes && box->fishes > 0) {
        //mvwprintw(mapWin, yOffset, xOffset+4, "\U0001f41f");
        for (int i = 0; i < box->fishes; ++i) {
            int yFish = i;
            int xFish;
            if (yFish == 1) {
                xFish = rand() % 2 + 1;
                if (xFish == 2) xFish = 6;
            } else {
                xFish = rand() % 4 + 2;
            }
            if (box->fishValues[i] == 1) mvwprintw(mapWin, yOffset + yFish + 1, xOffset + xFish, "\U0001f433");
            else if (box->fishValues[i] == 2) mvwprintw(mapWin, yOffset + yFish + 1, xOffset + xFish, "\U0001f41f");
            else mvwprintw(mapWin, yOffset + yFish + 1, xOffset + xFish, "\U0001f988");
        }
    }
    if (box->playerId >= 0) {
        // Print penguin and highlight box
        mvwprintw(mapWin, yOffset + 2, xOffset + 3, "\U0001f427");
        highlightBox(box, mapWin, box->playerId + 1);
    } else {
        // remove penguin and highlight
        mvwprintw(mapWin, yOffset + 2, xOffset + 3, " ");
        removeHighlightBox(box, mapWin);
    }
    wrefresh(mapWin);
}

/*
 *
 * Return 0 if no key was useful, 1 if a key was a movement and 2 if the key was enter.
 *
 */
int boxSelection(int key, Map* map, Coord* coord) {
    switch (key) {
        case KEY_UP:
        case 65: // up
            if (coord->x+1 == map->length && coord->y % 2 == 1)
                coord->y--;
            coord->y --;
            return 1;
        case KEY_DOWN:
        case 66: // down
            if (coord->x+1 == map->length && coord->y % 2 == 1)
                coord->y++;
            coord->y++;
            return 1;
        case KEY_RIGHT:
        case 67: // right
            coord->x += 1;
            return 1;
        case KEY_LEFT:
        case 68: // left
            coord->x -= 1;
            return 1;
        case KEY_ENTER:
        case 10:
            return 2;
        default:
            return 0;
    }
}