#Cplanarity

A simple algorithm based on the paper [Trémaux trees and planarity](https://www.sciencedirect.com/science/article/pii/S0195669811001600?via%3Dihub#br000060) by Hubert de Fraysseix and Patrice Ossona de Mendez. DS stores constraints as only a bitfield of lowpoints - graph size is limited to 64 vertices. Uses graph format from nauty, but should be easily adaptable. Currently includes both recursive (planarity0) and non-recursive (planarity1) main algorithms, both offering very similar performance (planarity1 slower by ~1%).
