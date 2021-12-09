//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

package mainproject;

network MainNet
{
    @display("bgb=504,292");
    submodules:
        coordinator: Coordinator {
            @display("p=194,194;i=block/cogwheel");
        }
        node: Node {
            @display("p=74,55;i=block/socket");
        }
        node1: Node {
            @display("p=353,62;i=block/socket");
        }
    connections:
        coordinator.out1 --> node1.coordIn;
        coordinator.out0 --> node.coordIn;
        node.dataOut --> node1.dataIn;
        node1.dataOut --> node.dataIn;
}
