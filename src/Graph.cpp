/*
 * Graph.cpp
 *
 *  Created on: 20 Aug 2014
 *      Author: dhf
 */

#include "Graph.h"
#include "Constants.h"
#include "LKVertex.h"
#include <string>
#include <algorithm>
#include <stdlib.h>
#include <time.h>
#include <algorithm>

Graph::Graph()
    {
    nVertices = 0;
    nTotalEdges = 0;
    nConnectedComponents = 0;
    vertices.clear();
    edges.clear();

    srand(time(NULL));

    }

Graph::Graph(int nVert)
    {
    nVertices = nVert;
    nTotalEdges = 0;
    nConnectedComponents = 0;
    vertices.clear();

    for (int i=0; i< nVertices; i++)
	{
	vertices.push_back(new Vertex(i));
	}

    }

Graph::Graph(vector<Vertex*> vert, vector<Edge*> edge)
    {

    vertices = vert;
    edges = edge;
    nConnectedComponents = 0;
    nVertices = vertices.size();
    nTotalEdges = edges.size();
    srand(time(NULL));

    }

Graph::~Graph()
    {

    // Need to be sure to delete the pointers associated with the graph

    /* for (int iVertex =0; iVertex< vertices.size();iVertex++)
     {
     delete (vertices[iVertex]);
     }
     vertices.clear();

     for (int iEdge =0; iEdge< edges.size();iEdge++)
     {
     delete (edges[iEdge]);
     }
     edges.clear();*/

    }

void Graph::clearGraph()

    {
    /*
     * Written 29/1/18 by dh4gan
     * Resets all Graph variables to zero
     *
     */

    clearComponentData();
    vertices.clear();
    edges.clear();
    nEdges.clear();

    nVertices = 0;
    nTotalEdges = 0;
    nConnectedComponents = 0;

    }

// Static methods used by Graph

static double randomReal()
    {
    // Returns a random real between zero and 1
    double number = (double) rand() / (double) RAND_MAX;
    return number;
    }

static double uniformSample(double min, double max)
    {
    return min + (max-min)*randomReal();
    }


static double gaussianSample(double mu, double sigma)
    {
  /*
   * Written 22/2/18 by dh4gan
   * samples a Gaussian using the Box-Muller method
   *
   */


    double w=10.0;
    double y1,y2;

    double min = 0.0;
    double max = 0.0;

    while(w>1.0)
      {
	y1 = uniformSample(min,max);
	y2 = uniformSample(min,max);

	w = y1*y1 + y2*y2;

      }

    // Get Gaussian N(0,1)
    double x = y1*sqrt(-log(w)/w);

    // Convert to N(mu, sigma^2)
    x = x*sigma + mu;

    return x;
    }

static Vector3D rotateIntoPlane(Vector3D vec, double argumentPeriapsis, double inclination, double longitudeAscendingNode)

    {
    /*
     * Written 29/1/18 by dh4gan
     * Given a vector aligned with the x-y plane, rotates it into the orbital plane defined by
     * argument of periapsis, inclination and longitude of the ascending node
     *
     */


    Vector3D newvec = vec;

    if(argumentPeriapsis!=0.0)
   	{
       newvec.rotateZ(-1 * argumentPeriapsis);
   	}

       /* Secondly, Rotate around x by -inclination */

       if(inclination !=0.0)
   	{
       newvec.rotateX(-1 * inclination);
   	}

       /* Lastly, Rotate around z by longitudeAscendingNode */

       if(longitudeAscendingNode !=0.0)
   	{
       newvec.rotateZ(-1 * longitudeAscendingNode);
   	}

       return newvec;

    }


static Vector3D calcPositionFromOrbit(double G, double totmass,double semiMajorAxis,
	double eccentricity, double inclination, double trueAnomaly,double longitudeAscendingNode,double argumentPeriapsis)
    {

    /* Author:dh4gan 1/8/13
     *
     * Calculates a position and velocity, given orbital elements (relative to CoM)
     * Uses separation and true anomaly to calculate position in frame coplanar with orbit
     * Then uses a series of rotations to give correct inclination and longitudes of periapsis and ascending nodes
     *
     */

    double magpos;
    Vector3D position;

    /* 1. calculate distance from CoM using semimajor axis, eccentricity and true anomaly*/

    magpos = semiMajorAxis * (1.0 - eccentricity * eccentricity) / (1.0
	    + eccentricity * cos(trueAnomaly));

    /* 2. Calculate position vector in orbital plane */

    position.elements[0] = magpos * cos(trueAnomaly);
    position.elements[1] = magpos * sin(trueAnomaly);
    position.elements[2] = 0.0;


    // Rotate into the orbital plane

    Vector3D newposition = rotateIntoPlane(position, argumentPeriapsis, inclination, longitudeAscendingNode);

    return newposition;

    }

// End of static methods



bool Graph::edgeInGraph(Edge* e)
    {
    return e->inVector(edges);
    }

bool Graph::vertexInGraph(Vertex* v)
    {
    return v->inVector(vertices);
    }

void Graph::addVertex(Vertex* v)
    {

    // Check Vertex is not in the Graph Already

    if (vertexInGraph(v) == false)
	{
	vertices.push_back(v);
	nVertices = int(vertices.size());
	v->setID(nVertices - 1);
	}

    }

void Graph::addVertex(Vertex* v, int index)
    {

    // Check Vertex is not in the Graph already
    if (vertexInGraph(v) == false)
	{
	vertices.push_back(v);
	nVertices = int(vertices.size());
	v->setID(index);
	}

    }

void Graph::addEdge(Edge* e)
    {

    // Check Edge not in the graph already

    if (edgeInGraph(e) == false)
	{

	edges.push_back(e);
	nTotalEdges = int(edges.size());
	}

    }

void Graph::removeEdge(int iEdge)
    {

    edges.erase(edges.begin() + iEdge);
    nTotalEdges = int(edges.size());
    }

void Graph::removeVertex(int iVertex)
    {

    Vertex* remove = vertices[iVertex];

    int nEdges = remove->getNEdges();

    // Remove the vertex from the vector of vertices
    vertices.erase(vertices.begin() + iVertex);
    nVertices = int(vertices.size());

    // If vertex has any edges, remove them
    if (nEdges > 0)
	{
	// Find all edges attached to the vertex
	// Flag for removal

	vector<bool> edgeAttachedToVertex(false, nEdges);

	for (int iEdge = nEdges - 1; iEdge >= 0; iEdge--)
	    {
	    if (edges[iEdge]->getBeginning() == remove
		    or edges[iEdge]->getEnd() == remove)
		{
		removeEdge(iEdge);
		}
	    }
	}

    }

Vertex* Graph::selectRandomVertex()
    {
    /*
     * Written 13/4/16 by dh4gan
     * Selects a vertex at random from the graph
     *
     */

    // Generate random number between 0 and nVertices-1
    int iVertex = rand() % (nVertices - 1);

    if (vertices[iVertex] == NULL)
	{
	return vertices[0];
	}
    else
	{
	return vertices[iVertex];
	}
    }

Vertex* Graph::findNearestVertex(Vector3D location)
    {

    Vertex* v, *nearestVertex;
    double minsep = 1.0e30;

    for (int iVertex = 0; iVertex < getNVertices(); iVertex++)
	{

	v = vertices[iVertex];

	Vector3D position = v->getPosition();

	double separation = position.subtractVector(location).magVector();

	if (separation < minsep)
	    {
	    minsep = separation;
	    nearestVertex = v;
	    }

	}

    return nearestVertex;

    }

int Graph::removeIsolatedVertices()
    {
    int iVertex;
    vector<bool> isolated(nVertices, false);

    int nIsolated = 0;
    // Test to see which vertices are isolated
    for (iVertex = 0; iVertex < nVertices; iVertex++)
	{
	//printf("%i %i %i\n", iVertex, nVertices, vertices[iVertex]->getNEdges());
	if (vertices[iVertex]->getNEdges() == 0)
	    {

	    isolated[iVertex] = true;
	    nIsolated++;
	    }
	}

    // Remove all isolated Vertices
    for (iVertex = nVertices - 1; iVertex >= 0; iVertex--)
	{
	if (isolated[iVertex])
	    {
	    removeVertex(iVertex);
	    }

	}

    nVertices = int(vertices.size());

    return nIsolated;
    }

void Graph::addGraph(Graph other)
    {

    /*
     * Written 13/4/16 by dh4gan
     * Adds vertices and edges of graph 'other'
     * to current graph object
     * Tests for duplicate Vertex and Edge objects
     *
     */

    Vertex *v;
    Edge *e;

    vector<Vertex*> otherVertices = other.getVertices();
    vector<Edge*> otherEdges = other.getEdges();

    int nVertexOther = otherVertices.size();
    int nEdgeOther = otherEdges.size();
    // Add vertices (if they're not already present)
    for (int iVertex = 0; iVertex < nVertexOther; iVertex++)
	{
	v = otherVertices[iVertex];
	if (not (v->inVector(vertices)))
	    {
	    addVertex(v);
	    }
	}

    // Add edges (if they're not already present)

    for (int iEdge = 0; iEdge < nEdgeOther; iEdge++)
	{
	e = otherEdges[iEdge];
	if (not (e->inVector(edges)))
	    {
	    addEdge(e);
	    }
	}

    // Update other graph properties
    nVertices = vertices.size();
    nTotalEdges = edges.size();

    // Find new connected components
    findConnectedComponents();

    }

void Graph::clearComponentData()

    {

    /*
     * Written 12/4/16 by dh4gan
     * Clears all the data regarding connected components
     *
     */

    for (int iVertex = 0; iVertex < nVertices; iVertex++)
	{
	vertices[iVertex]->setComponentID(0);
	}
    nConnectedComponents = 0;

    nInComponent.clear();

    }

void Graph::resetVertexWeights()

    {
    /*
     * Written 16/4/16 by dh4gan
     * Resets all the Vertex weights
     *
     */

    for (int iVertex = 0; iVertex < nVertices; iVertex++)
	{
	vertices[iVertex]->setWeight(0.0);
	}

    }

void Graph::calculateEdgeDistanceWeights()
    {

    /*
     * Written 15/4/16 by dh4gan
     * Updates all Edge weights in graph according to distance
     * between Vertices
     */

    for (int iEdge = 0; iEdge < nTotalEdges; iEdge++)
	{

	edges[iEdge]->calcDistanceWeight();
	//printf("%f \n",edges[iEdge]->getWeight());
	}

    }

double Graph::calculateTotalEdgeWeight()

    {
    /* Written 3/6/16 by dh4gan
     *
     * Calculates the total edge weight in the Graph
     *
     */

    double totalWeight = 0.0;

    for (int iEdge = 0; iEdge < nTotalEdges; iEdge++)
	{
	totalWeight = totalWeight + edges[iEdge]->getWeight();
	}

    return totalWeight;
    }

void Graph::findConnectedComponents(Vertex* first)

    {
    /*
     * Written 11/4/16 by dh4gan
     * Calculates the number of connected components to the graph
     * Also records a single vertex from each graph
     * Uses breadth-first search
     *
     */

    Vertex *start, *v;
    vector<Vertex*> connectedVertices, connectedToVertex;
    vector<Edge*> edges;

    // Clear out old component data
    clearComponentData();

    connectedVertices.reserve(nVertices); // Reserve memory in case graph is fully connected

    // Pick the starting vertex

    start = first;

    int iVertex = 0;
    //int nInComponent = 0;
    int nCounted = 0;

    // Count connected components until number of vertices counted is equal to total number of vertices

    while (nCounted < nVertices)
	{

	// Start a new component, with "start" identified as the centre of the component
	nConnectedComponents = nConnectedComponents + 1;
	start->setComponentID(nConnectedComponents);

	//printf("Component %i has centre %p, edge count %i \n", nConnectedComponents, start, int(start->getConnectedVertices().size()));
	componentCentres.push_back(start);

	// Now find all vertices connected to start

	connectedVertices.push_back(start);

	iVertex = 0;
	nInComponent.push_back(1);

	//printf("%i \n", nInComponent[nConnectedComponents-1]);
	// Now go through connectedVertices,
	// Adding all vertices connected to any entry in connectedVertices

	while (iVertex < nInComponent[nConnectedComponents - 1])
	    {
	    // Get all vertices directly connected to current vertex
	    connectedToVertex =
		    connectedVertices[iVertex]->getConnectedVertices();

	    int nConnectedToVertex = connectedToVertex.size();

	    // If they're not already present and not in another component, add them to vector
	    for (int jVertex = 0; jVertex < nConnectedToVertex; jVertex++)
		{
		v = connectedToVertex[jVertex];

		if (not (v->inVector(connectedVertices))
			and v->getComponentID() == 0)
		    {
		    connectedVertices.push_back(v);
		    v->setComponentID(nConnectedComponents);
		    }
		}

	    nInComponent[nConnectedComponents - 1] = connectedVertices.size();
	    iVertex++;
	    }

	// Add total connected in this component to the final count

	nCounted = nCounted + nInComponent[nConnectedComponents - 1];

	//printf("Component %i has %i members \n", nConnectedComponents, nInComponent[nConnectedComponents-1]);
	// Record the total number of vertices in the component

	connectedVertices.clear();
	// If there are still unconnected vertices, then find the next vertex to begin from
	if (nCounted < nVertices)
	    {
	    for (int iVertex = 0; iVertex < nVertices; iVertex++)
		{
		if (vertices[iVertex]->getComponentID() == 0)
		    {
		    start = vertices[iVertex];
		    break;
		    }
		}

	    }
	//printf("Number in Component %i: %i \n", nConnectedComponents, nInComponent[nConnectedComponents]);
	}

    }

void Graph::findConnectedComponents()

    {
    /*
     * Written 12/4/16
     * Overloaded method to find connected components
     * Selects a vertex at random to begin
     */

    Vertex* random = selectRandomVertex();
    findConnectedComponents(random);

    }

void Graph::findConnectedComponents(int iVertex)
    {

    Vertex* v = vertices[iVertex];
    findConnectedComponents(v);
    }

Graph Graph::minimumSpanningTree(Vertex* start)
    {
    /*
     * Written 8/4/16 by dh4gan
     * Uses Prim's Algorithm to find the
     * minimum spanning tree beginning at Vertex start
     * If connected component number greater than 1,
     * Will only return MST for component containing start
     *
     */

    int iVertex, iEdge;
    Vertex* v, *beginningVertex, *endingVertex;
    Edge* e, *nearestEdge;

    vector<Edge*> v_edges;
    vector<Vertex*> v_vertices;

    double minweight;

    // Check which component we are calculating MST for

    int component = start->getComponentID();

    // Add starting vertex to Graph

    Graph tree;
    tree.addVertex(start);

    // Begin While loop

    //double percent = 0.0;
    //double outputpercent = 10.0;
    while (tree.getNVertices() < nInComponent[component - 1])
	{

	/*percent = 100.0*(float)tree.getNVertices()/ (float) nInComponent[component-1];
	 if(percent > outputpercent)
	 {
	 printf("%3.0f %% complete \n", percent);
	 outputpercent = outputpercent +10.0;
	 }*/
	// Search Graph to find connected edge with minimum weight
	minweight = 1.0e30;
	nearestEdge = NULL;

	// Test all edges associated with vertices in the forest, and find smallest weight
	for (iVertex = 0; iVertex < tree.getNVertices(); iVertex++)
	    {

	    // Take a vertex from the tree
	    v = tree.getVertex(iVertex);
	    //printf("iVertex: %i \n",iVertex);
	    // Get its edges and connected vertices

	    v_edges = v->getEdges();
	    v_vertices = v->getConnectedVertices();

	    int nv_edges = v_edges.size();
	    // Find edge with minimum weight
	    for (iEdge = 0; iEdge < nv_edges; iEdge++)
		{

		// Take an edge which connects to this vertex

		e = v_edges[iEdge];

		// If edge already in tree, skip it
		if (tree.edgeInGraph(e))
		    {
		    continue;
		    }

		// If edge not connecting to a new vertex, skip it
		beginningVertex = e->getBeginning();
		endingVertex = e->getEnd();

		if (tree.vertexInGraph(beginningVertex)
			and tree.vertexInGraph(endingVertex))
		    {
		    continue;
		    }

		//printf("iVertex: %i %p %p %f %d\n",iVertex, e, nearestEdge, e->getWeight(),tree.edgeInGraph(e));

		if (e->getWeight() < minweight)
		    {
		    minweight = e->getWeight();
		    nearestEdge = e;
		    //nearestVertex = v;
		    }
		}
	    }
	// End of loop over edges

	// If a valid edge exists, add it to the Graph
	if (nearestEdge)
	    {

	    beginningVertex = nearestEdge->getBeginning();
	    if (not (tree.vertexInGraph(beginningVertex)))
		{
		tree.addVertex(beginningVertex);
		}

	    endingVertex = nearestEdge->getEnd();
	    if (not (tree.vertexInGraph(endingVertex)))
		{
		tree.addVertex(endingVertex);
		}

	    tree.addEdge(nearestEdge);
	    }

	}

    return tree;
    }

Graph Graph::minimumSpanningTree()

    {
    /*
     * Written 11/4/16 by dh4gan
     * Randomly selects a Vertex, and initiates MST using above method
     *
     */

    Vertex* random = selectRandomVertex();
    printf("%p selected at random for minimumSpanningTree \n", random);
    Graph tree = minimumSpanningTree(random);

    return tree;

    }

Graph Graph::minimumSpanningForest()
    {

    Graph forest, tree;

    // Get connected components
    findConnectedComponents();


    printf("Computing Minimum Spanning Forest with %i components \n", nConnectedComponents);

    for (int iVertex = 0; iVertex < nConnectedComponents; iVertex++)
	{
	//printf("Calculating minimum spanning tree for component %i \n", iVertex+1);

	Vertex* icomp = componentCentres[iVertex];
	// For each vertex that founds a connected component, calculate MST
	tree = minimumSpanningTree(icomp);

	/*printf("Tree calculated for %p: nVertices %i, nEdges %i \n",
	 componentCentres[iVertex], tree.getNVertices(),
	 tree.getTotalEdges());*/

	// add each MST to a single Graph
	forest.addGraph(tree);

	}

    return forest;

    }

//Graph Graph::minimumSpanningForest()
//{
//    /*
//     * Written 20/8/14 by dh4gan
//     * Returns a minimum spanning forest from the Graph object
//     * This assumes the object has fully populated weights, and is undirected
//     * This is designed for unconnected graphs
//     * Returns a Graph containing all vertices, and the minimum number of edges to connect them
//     */
//
//    int iVertex, iEdge;
//    int fVertex, fEdge;
//    Vertex* v, *nearestVertex, *beginningVertex, *endingVertex;
//    Edge* e, *nearestEdge;
//
//    vector<Edge*> v_edges;
//    vector<Vertex*> v_vertices;
//
//    double minweight;
//
//    // Create the forest (initially empty)
//    Graph forest;
//
//    // Find all vertices with only one edge, and add them to the forest
//
//    for(iVertex=0; iVertex<getNVertices(); iVertex++)
//	{
//	if(vertices[iVertex]->getNEdges()==1) forest.addVertex(vertices[iVertex]);
//	}
//
//    // If there are no vertices with one edge, then add the first vertice with any edges at all
//
//    if(forest.getNVertices()==0)
//	{
//	iVertex = 0;
//
//	while(vertices[iVertex]->getNEdges()==0)
//	    {
//	    iVertex++;
//	    }
//
//	forest.addVertex(vertices[iVertex]);
//
//	}
//
//
//    // Add all edges associated with vertices in the forest so far (and any vertices connected)
//
//    int nOneEdge = forest.getNVertices();
//
//    for (fVertex=0;fVertex< nOneEdge; fVertex++)
//	{
//
//	v = forest.getVertex(fVertex);
//
//	//printf("Testing %i %i %i\n", fVertex, v->getNEdges(), forest.getNVertices());
//
//	v_edges = v->getEdges();
//	v_vertices = v->getConnectedVertices();
//
//	// Add the smallest edge
//
//	minweight = 1.0e30;
//
//	if(v->getNEdges() >1)
//	    {
//	for (fEdge=0; fEdge< v->getNEdges(); fEdge++)
//	    {
//
//	    e = v_edges[fEdge];
//
//	    if(e->getWeight() < minweight)
//		{
//		minweight = e->getWeight();
//		nearestEdge = e;
//		}
//
//	    }
//
//	    }
//	else
//	    {
//	    nearestEdge = v_edges[0];
//	    }
//
//	forest.addEdge(nearestEdge);
//
//	// Find the vertex connected to this edge
//
//	beginningVertex = findNearestVertex(nearestEdge->getBeginning());
//	forest.addVertex(beginningVertex);
//
//	endingVertex = findNearestVertex(nearestEdge->getEnd());
//	forest.addVertex(endingVertex);
//
//
//	}
//
//
//    // Begin loop (loop ends when all vertices in the tree)
//
//    while(forest.getNVertices() < getNVertices())
//	{
//
//	 minweight = 1.0e30;
//	 nearestEdge = NULL;
//	 nearestVertex = NULL;
//
//	// Test all edges associated with vertices in the forest, and find smallest weight
//	for (iVertex = 0; iVertex < forest.getNVertices(); iVertex++)
//	    {
//
//	    // Take a vertex from the tree
//	    v = forest.getVertex(iVertex);
//
//	    // Get its edges and connected vertices
//
//	    v_edges = v->getEdges();
//	    v_vertices = v->getConnectedVertices();
//
//	    // Find edge with minimum weight
//	    for (iEdge = 0; iEdge < v_edges.size(); iEdge++)
//		{
//
//		// Take an edge which connects to this vertex
//
//		e = v_edges[iEdge];
//
//		if (forest.edgeInGraph(e) == false)
//		    {
//		    // Compare weights
//		     if (e->getWeight() < minweight)
//			{
//			minweight = e->getWeight();
//			nearestEdge = e;
//			nearestVertex = v;
//			}
//		    }
//		}
//		// End of loop over edges
//
//
//	    }
//	    // End of loop over vertices
//
//	    if(nearestEdge) // Testing that this isn't a null pointer
//	    {
//
//	    //printf("Edge %p, with minweight %f \n", nearestEdge, minweight);
//
//	    // If this edge isn't in the tree, add it
//
//	    forest.addEdge(nearestEdge);
//
//	    // Add the vertices associated with it (if they're not in the forest)
//
//	    // Start with vertex at the beginning of the edge
//	    nearestVertex = findNearestVertex(nearestEdge->getBeginning());
//	    forest.addVertex(nearestVertex);
//
//	    // Now the vertex at the end of the edge
//	    nearestVertex = findNearestVertex(nearestEdge->getEnd());
//	    forest.addVertex(nearestVertex);
//	    }
//
//	else
//	    {
//
//	    // Add a vertex at random from the main graph
//
//	    iVertex = rand() % nVertices;
//
//	     forest.addVertex(vertices[iVertex]);
//
//	    }
//
//	}
//
//    printf("Graph Total Edges %i\n", getTotalEdges());
//    printf("Forest Total Edges %i\n", forest.getTotalEdges());
//    return forest;
//
//
//}

Graph Graph::shortestPath(Vertex* start, Vertex* destination, double &distance)
    {
    /*
     * Written 29/3/16 by dh4gan
     * Method returns a graph showing the shortest path
     * between the vertices start and destination
     * Uses the A* algorithm to determine which vertices
     * to add to the path
     *
     */

    vector<Vertex*> neighbours, pathVertices, pathVertexCopies;
    vector<Edge*> neighbourEdges, pathEdges;
    Vertex* neighbourNode, *currentNode, *nextNode;
    double goalDistance, minWeight, newWeight;

    // Check that both Vertices are in the same connected component
    // Return negative value if they aren't connected

    if (start->getComponentID() != destination->getComponentID())
	{
	printf(
		"Starting and Finishing Vertex are not connected, returning empty path \n ");
	distance = -10.0;
	Graph emptyGraph;
	return emptyGraph;

	}

    distance = 0.0;

    // Set all weights to zero
    resetVertexWeights();

    // Reset all visited markers

    for (int iVertex = 0; iVertex < nVertices; iVertex++)
	{
	vertices[iVertex]->setVisited(false);
	}

    // Define start node to be current node
    currentNode = start;

    // Add start node to list of vertices on the shortest path
    pathVertices.push_back(start);

    // set its weight to zero
    currentNode->setWeight(0.0);
    distance = 0.0;

    printf("Start: %i, %p %i \n", start->getID(), start, start->getNEdges());
    printf("Destination: %i, %p \n", destination->getID(), destination);

    // Begin while loop
    while (currentNode != destination)
	{

	currentNode->setVisited(true);

	// Loop over edges

	minWeight = 1.0e30;

	neighbourEdges = currentNode->getEdges();

	// Check for a dead end - occurs if current node only has one edge

	if (currentNode->getNEdges() == 1 and currentNode != start)
	    {

	    bool newRoute = false;

	    printf("Back tracking \n");
	    // Back tracking algorithm:
	    // Do until (nEdges>2 and a node is unexplored (weight=0))
	    // Pop last entry in pathVertices, set its weight to 1e30

	    while (not (newRoute))
		{
		Vertex* lastVertex = pathVertices.back();

		if (lastVertex != start)
		    {

		    lastVertex->setWeight(1.0e30);
		    lastVertex->setVisited(false);
		    pathVertices.pop_back();
		    }

		lastVertex = pathVertices.back();

		// If node has more than two edges, check for a non-dead-end vertex
		newRoute = lastVertex->getNEdges() > 2 or lastVertex == start;

		if (newRoute)
		    {

		    bool deadend = true;

		    neighbourEdges = lastVertex->getEdges();
		    for (int iEdge = 0; iEdge < lastVertex->getNEdges();
			    iEdge++)
			{
			// Find Vertex connected to this edge
			neighbourNode = neighbourEdges[iEdge]->getBeginning();
			if (neighbourNode == lastVertex)
			    {
			    neighbourNode = neighbourEdges[iEdge]->getEnd();
			    }

			if (neighbourNode->getWeight() < 1.0e30)
			    {
			    deadend = false;
			    break;
			    }

			}

		    // If there is a non dead end vertex, stop the back-tracking here
		    if (deadend)
			{
			newRoute = false;
			}
		    else
			{
			newRoute = true;
			currentNode = lastVertex;
			}

		    }

		if (lastVertex == start)
		    {
		    printf("Back-tracked back to the start\n");
		    newRoute = true;
		    //exit(EXIT_FAILURE);

		    }
		}

	    }
	// End of back-tracking algorithm

	// Find next vertex in the shortest path

	for (int iEdge = 0; iEdge < currentNode->getNEdges(); iEdge++)
	    {

	    // Find Vertex connected to this edge
	    neighbourNode = neighbourEdges[iEdge]->getBeginning();
	    if (neighbourNode == currentNode)
		{
		neighbourNode = neighbourEdges[iEdge]->getEnd();
		}

	    // Distance from neighbour Vertex to the goal
	    goalDistance = neighbourNode->calcVertexSeparation(destination);

	    // Update its weight with the weight of the current node + edge weight + distance to goal

	    printf("%i %i %p %f %f \n", currentNode->getID(),
		    neighbourNode->getID(), nextNode, goalDistance,
		    neighbourEdges[iEdge]->getWeight());

	    newWeight = neighbourNode->getWeight() + currentNode->getWeight()
		    + neighbourEdges[iEdge]->getWeight() + goalDistance;

	    neighbourNode->setWeight(newWeight);

	    // If this neighbour has the smallest weight, it's next in line
	    if (newWeight < minWeight and not (neighbourNode->isVisited()))
		{

		nextNode = neighbourNode;

		minWeight = newWeight;
		}

	    if (nextNode)
		{
		printf(
			"currentNode, neighbourNode, nextNode: %i %i %i %f %f \n",
			currentNode->getID(), neighbourNode->getID(),
			nextNode->getID(), newWeight, minWeight);

		currentNode->getPosition().printVector();
		nextNode->getPosition().printVector();
		printf("%i \n", nextNode->getNEdges());
		}

	    }

	//exit(EXIT_FAILURE);
	if (nextNode)
	    {

	    // Add this vertex to the path

	    pathVertices.push_back(nextNode);
	    currentNode = nextNode;

	    nextNode = 0;
	    }
	else
	    {
	    printf("next node not found - dead end? \n");
	    distance = -5.0;
	    return Graph(pathVertexCopies, pathEdges);
	    }

	// End of loop over currentNode
	}

    // Reset all visited markers

    for (int iVertex = 0; iVertex < nVertices; iVertex++)
	{
	vertices[iVertex]->setVisited(false);
	}

    // Now construct graph from the list of path vertices
    // We construct a brand new graph, so create copies of vertices with the same positions

    int nPathVertices = pathVertices.size();
    for (int iVertex = 0; iVertex < nPathVertices; iVertex++)
	{
	pathVertexCopies.push_back(
		new Vertex(pathVertices[iVertex]->getPosition()));
	}

    // vertices vector is in the correct order, so Edge construction is straightforward

    for (int iVertex = 1; iVertex < nPathVertices; iVertex++)
	{

	// Create new Edge based on two adjacent vertices in list
	pathEdges.push_back(
		new Edge(pathVertexCopies[iVertex - 1],
			pathVertexCopies[iVertex]));

	// Add distance between both edges to distance calculator
	distance = distance + pathEdges.back()->getWeight();

	// Add this Edge to the internal storage of both Vertex objects

	pathVertexCopies[iVertex - 1]->addConnectedEdge(pathEdges.back(),
		pathVertexCopies[iVertex]);
	pathVertexCopies[iVertex]->addConnectedEdge(pathEdges.back(),
		pathVertexCopies[iVertex - 1]);
	}

    return Graph(pathVertexCopies, pathEdges);
    }

void Graph::readFromFile(string &inputFileString)
    {
    /*
     * Reads the graph from file in a matrix format
     * Records all vertices, and connections between vertices
     * Matrix entries Mij are the weights of the edges between (i,j)
     * Undirected graphs have Mij = Mji
     * Directed graphs have Mij = edge begins at i
     */

    int vertexNumber = 0;
    int edgeNumber = 0;
    int ID = 0;
    int iVertex, jVertex, nConnected;

    FILE *inputFile = fopen(inputFileString.c_str(), "r");

    // Read first line from inputfile to get Vertex Number

    fscanf(inputFile, "%i %i %i \n", &vertexNumber, &edgeNumber, &nConnected);

    printf("Vertices: %i\n Edges:%i\n", vertexNumber, edgeNumber);
    // Create output matrix, and read it from the input file

    float matrix[vertexNumber][vertexNumber];

    for (int iVertex = 0; iVertex < vertexNumber; iVertex++)
	{

	// Read the Vertex Position data

	fscanf(inputFile, "%i", &ID);

	Vector3D position;
	fscanf(inputFile, "%F   %F   %F  ", &position.elements[0],
		&position.elements[1], &position.elements[2]);

	// Create new Vertex, and add to the Graph object
	Vertex* v = new Vertex(position);

	addVertex(v, ID);

	// Now read the matrix
	for (int jVertex = 0; jVertex < vertexNumber; jVertex++)
	    {
	    fscanf(inputFile, "%f ", &matrix[iVertex][jVertex]);

	    }

	}

    // Create edges according to the matrix values

    for (iVertex = 0; iVertex < vertexNumber; iVertex++)
	{
	for (jVertex = 0; jVertex < vertexNumber; jVertex++)
	    {
	    // If matrix non-zero, create an edge between the two vertices in the array

	    if (matrix[iVertex][jVertex] > 0.0)
		{
		Edge* e = new Edge(getVertex(iVertex), getVertex(jVertex),
			matrix[iVertex][jVertex]);

		vertices[iVertex]->addConnectedEdge(e, vertices[jVertex]);
		vertices[jVertex]->addConnectedEdge(e, vertices[iVertex]);

		addEdge(e);

		}

	    }
	}

    fclose(inputFile);

    }

void Graph::writeToFile(string &outputFileString)
    {
    /*
     * Writes the graph to file in a matrix format
     * Records all vertices, and connections between vertices
     * Matrix entries Mij are the weights of the edges between (i,j)
     * Undirected graphs have Mij = Mji
     * Directed graphs have Mij = edge begins at i
     */
    int iVertex, jVertex;
    int iEdge, iBegin, iEnd;
    Edge* e;
    Vertex* begin, *end;
    vector<Vertex*>::iterator itBegin, itEnd;
    string outputString;

    FILE* outputFile = fopen(outputFileString.c_str(), "w");

    // Create output matrix

    double matrix[nVertices][nVertices];

    for (int iVertex = 0; iVertex < nVertices; iVertex++)
	{
	for (int jVertex = 0; jVertex < nVertices; jVertex++)
	    {
	    matrix[iVertex][jVertex] = 0.0;
	    }

	}

    for (iEdge = 0; iEdge < nTotalEdges; iEdge++)
	{
	e = edges[iEdge];

	// Find Beginning and Ending Vertices
	begin = e->getBeginning();
	end = e->getEnd();

	// Find them in Vertex Array

	itBegin = find(vertices.begin(), vertices.end(), begin);
	itEnd = find(vertices.begin(), vertices.end(), end);

	iBegin = distance(vertices.begin(), itBegin);
	iEnd = distance(vertices.begin(), itEnd);

	// Add the weight of this edge to the matrix
	matrix[iBegin][iEnd] = e->getWeight();

	}

    // Write header line giving the number of vertices and edges
    fprintf(outputFile, "%i %i %i \n", nVertices, nTotalEdges,
	    nConnectedComponents);

    for (iVertex = 0; iVertex < nVertices; iVertex++)
	{
	outputString = "";

	// Skip Vertices without any edges

	//if(nEdges[iVertex]==0) {continue;}

	fprintf(outputFile, "%i ", iVertex + 1);
	// Start with star's position
	fprintf(outputFile, "%+.4E   %+.4E   %+.4E  ",
		vertices[iVertex]->getPosition().elements[0],
		vertices[iVertex]->getPosition().elements[1],
		vertices[iVertex]->getPosition().elements[2]);

	// Now look through all edges for this vertex

	for (jVertex = 0; jVertex < nVertices; jVertex++)
	    {

	    fprintf(outputFile, "%+.4E  ", matrix[iVertex][jVertex]);

	    }
	// End of line
	fprintf(outputFile, "  \n");

	}

    fclose(outputFile);
    }

void Graph::clearAllEdges()
    {
    /*
     * Written 29/9/17 by dh4gan
     * Clears all the edges of all vertices in the Graph
     */

    for (int i = 0; i < nVertices; i++)

	{
	vertices[i]->clearEdges();
	}

    }

void Graph::createNeighbourNetwork(double range)

    {

    /*
     * Written 29/9/17 by dh4gan
     * Deletes all current edges, produces a new network where each Vertex connected to all Vertex objects within range
     *
     */

    double distance;
    double percent,timecounter;
    // Wipe previous edge catalogues
    clearAllEdges();

    // Now begin adding edges based on separation

    printf("Creating Nearest Neighbour Network, range: %f \n",range);
    printf("Vertices: %i \n",nVertices);

    for (int i = 0; i < nVertices; i++)
	{

	percent = percent + 100.0 / float(nVertices);
	   	if (percent > 10.0)
	   	    {
	   	    timecounter = timecounter + 10.0;
	   	    printf("%.0f %% complete \n", timecounter);
	   	    percent = 0.0;
	   	    }

	for (int j = i+1; j < nVertices; j++)
	    {
	    distance = vertices[i]->calcVertexSeparation(vertices[j]);
	    //printf("distance %f, range %f \n",distance,range);
	    // If within range, connect the vertices
	    if (distance < range)
		{
		// Add Edges to the graph
		Edge* e1 = new Edge(vertices[i], vertices[j], distance);

		Edge* e2 = new Edge(vertices[j],vertices[i],distance);

		// Add Edge to each vertex's catalogue of edges

		vertices[i]->addConnectedEdge(e1,vertices[j]);
		vertices[j]->addConnectedEdge(e2,vertices[i]);

		// Finally, add edge to graph
		addEdge(e1);
		addEdge(e2);

		}
	    }

	}

    printf("Total Number of Edges: %i \n",nTotalEdges);
    findConnectedComponents();


    }

void Graph::generateGHZ(int &iseed, int &nVertices, double &innerRadius, double &outerRadius, double &scale){



    int success;
    double eccmax, eccmax2,incmax;
    double inc, semimaj, ecc, meanAnom, argPer, longAscend;
    double periastron, apastron, percent, timecounter;
    string bodyType;

    printf("Generating GHZ initial conditions for %i stars\n",nVertices);
    printf("Inner Radius: %f, Outer Radius: %f, scale length: %f \n",innerRadius,outerRadius, scale);

    // All stars have the same mass
    double totalmass = float(nVertices);
    double G = Gmkpc; // G in kpc^3 Msol^{-1} kpc^{-2}

    srand(iseed);

       // Assert maximum eccentricity and inclination
       eccmax = 0.7;
       incmax = 0.5;

       // Scale surface density distribution for the range of radii involved

       double sigma_0 = (exp(-innerRadius/scale) - exp(-outerRadius/scale));

       // Randomly assign vertex positions

       percent = 0.0;
       timecounter = 0.0;

       for (int i = 0; i < nVertices; i++)
   	{

   	percent = percent + 1 / float(nVertices);
   	if (percent > 0.1)
   	    {
   	    timecounter = timecounter + 10.0;
   	    printf("%.0f %% complete \n", timecounter);
   	    percent = 0.0;
   	    }

   	// Assign semimajor axis such that the surface density profile is correct

   	semimaj = exp(-innerRadius/scale) -randomReal()*sigma_0;
   	semimaj = -log(semimaj)*scale;

   	// This defines maximum eccentricity the body can have to avoid going inside inner radius
   	eccmax = outerRadius/semimaj - 1.0;
   	eccmax2 = 1.0 - innerRadius/semimaj;

   	if(eccmax2 < eccmax) eccmax = eccmax2;

   	success = 0;

   	// ensure that eccentricity guarantees orbit entirely within GHZ
   	while (success == 0)
   	    {
   	    ecc = randomReal() *eccmax;

   	    periastron = semimaj * (1.0 - ecc);
   	    apastron = semimaj * (1.0 + ecc);

   	    success = 1;

   	    if (periastron < innerRadius)
   		{
   		success = 0;
   		}

   	    if (apastron > outerRadius)
   		{
   		success = 0;
   		}

   	    }

   	inc = -incmax + randomReal() * 2.0 * incmax;
   	meanAnom = randomReal() * 2.0 * pi;
   	argPer = randomReal() * 2.0 * pi;
   	longAscend = randomReal() * 2.0 * pi;

   	// Compute Position from this data

   	Vector3D pos = calcPositionFromOrbit(G,totalmass, semimaj,ecc,inc,meanAnom,argPer,longAscend);

   	addVertex(new LKVertex(i+1,pos));

   	}

}


void Graph:: generateCluster(int &iseed, int &nVert, double &rmax){

    /*
     * Written 29/1/18 by dh4gan
     *
     * Generates a Plummer Sphere
     * for Vertex positions
     *
     */

    clearGraph();

    //nVertices = nVert;

    double rmin = 0.0;
    double r =0.0;

    double thetamin = -pi/2;
    double thetamax = pi/2;

    double phimin = 0.0;
    double phimax = 2.0*pi;
    int success = 0;

    double percent = 0.0;
    double timecounter= 0.0;

    srand(iseed);

    printf("Generating Plummer Sphere Initial Conditions, Rmax: %f \n",rmax);
    printf("nVertices: %i \n",nVert);

    for (int i=0; i<nVert; i++)
	{

	percent = percent + 100.0 / float(nVert);
   	if (percent > 10.0)
   	    {
   	    timecounter = timecounter + 10.0;
   	    printf("%.0f %% complete \n", timecounter);
   	    percent = 0.0;
   	    }

   	// Randomly sample r from Plummer distribution
	success = 0;
	r = 0;
	while(success==0)
	    {
	    r = uniformSample(rmin,rmax);

	    // (Normalised) density profile of a Plummer sphere (1,0.17)
	    double rho = pow(1 + r*r/(rmax*rmax),-2.5);

	    double randtest = uniformSample(0.0,1);
	    if (rho>randtest)
		{
		success=1;
		}
	}

	double theta = uniformSample(thetamin,thetamax);
	double phi = uniformSample(phimin,phimax);

	double x = r*sin(theta)*cos(phi);
	double y = r*sin(theta)*sin(phi);
	double z = r*cos(theta);

	Vector3D pos(x,y,z);
	addVertex(new LKVertex(i+1,pos));

	}

}







void Graph::generateConstantLKParameters(double initialPrey, double initialPred, double preyGrow,
	double preyDeath,double predGrow, double predDeath, double preyCarry, double predCarry,double mutate, double outflow, double velocity, double t0)
    {
    /*
     * Written 29/1/18 by dh4gan
     * Initialises LK systems with globally constant parameters
     *
     */


    for (int i=0; i<nVertices; i++)
   	{


	// Only first vertex is given non-zero starting values of predators/prey
	if(i!=0)
	    {
	    initialPrey=0.0;
	    initialPred=0.0;
	    }

	vertices[i]->setLKParameters(initialPrey, initialPred, preyGrow,preyDeath,predGrow,predDeath,preyCarry,predCarry,mutate,outflow,velocity,t0);

   	}

    }

void Graph::generateUniformLKParameters(double initialPrey, double initialPred,
	double preyGrowMin, double preyGrowMax,
	double preyDeathMin, double preyDeathMax,
	double predGrowMin, double predGrowMax,
	double predDeathMin, double predDeathMax,
	double preyCarryMin, double preyCarryMax,
	double predCarryMin, double predCarryMax,
	double mutateMin, double mutateMax,
	double outflowMin, double outflowMax,
	double velocityMin, double velocityMax)

    {
    /*
     * Written 29/1/18 by dh4gan
     * Initialises LK systems with parameters sampled from uniform distribution
     *
     */


    double t0 = 0.0;

    for (int i=0; i<nVertices; i++)
      	{

	double preyGrow = uniformSample(preyGrowMin, preyGrowMax);
	double preyDeath = uniformSample(preyDeathMin, preyDeathMax);

	double predGrow = uniformSample(predGrowMin, predGrowMax);
	double predDeath = uniformSample(predDeathMin, predDeathMax);

	double preyCarry = uniformSample(preyCarryMin,preyCarryMax);
	double predCarry = uniformSample(predCarryMin,predCarryMax);

	double mutate = uniformSample(mutateMin, mutateMax);
	double outflow = uniformSample(outflowMin, outflowMax);
	double velocity = uniformSample(velocityMin, velocityMax);

	// Only first vertex is given non-zero starting values of predators/prey
   	if(i!=0)
   	    {
   	    initialPrey=0.0;
   	    initialPred=0.0;
   	    }

   	vertices[i]->setLKParameters(initialPrey, initialPred, preyGrow,preyDeath,predGrow,predDeath,preyCarry,predCarry,mutate,outflow,velocity,t0);

      	}


    }

void Graph::generateGaussianLKParameters(double initialPrey, double initialPred,
	double meanPreyGrow, double sdPreyGrow,
	double meanPreyDeath, double sdPreyDeath,
	double meanPredGrow, double sdPredGrow,
	double meanPredDeath, double sdPredDeath,
	double meanPreyCarry, double sdPreyCarry,
	double meanPredCarry, double sdPredCarry,
	double meanMutate, double sdMutate,
	double meanOutflow, double sdOutflow,
	double meanVelocity, double sdVelocity)

    {
    /*
     * Written 29/1/18 by dh4gan
     * Initialises LK systems with parameters sampled from Gaussian distribution
     *
     */

    double t0 = 0.0;

    for (int i=0; i<nVertices; i++)
      	{

	double preyGrow = gaussianSample(meanPreyGrow, sdPreyGrow);
	double preyDeath = gaussianSample(meanPreyDeath, sdPreyDeath);

	double predGrow = gaussianSample(meanPredGrow, sdPredGrow);
	double predDeath = gaussianSample(meanPredDeath, sdPredDeath);

	double preyCarry = gaussianSample(meanPreyCarry, sdPreyCarry);
	double predCarry = gaussianSample(meanPredCarry, sdPredCarry);

	double mutate = gaussianSample(meanMutate, sdMutate);
	double outflow = gaussianSample(meanOutflow, sdOutflow);
	double velocity = gaussianSample(meanVelocity, sdVelocity);

	// Only first vertex is given non-zero starting values of predators/prey
   	if(i!=0)
   	    {
   	    initialPrey=0.0;
   	    initialPred=0.0;
   	    }

   	vertices[i]->setLKParameters(initialPrey, initialPred, preyGrow,preyDeath,predGrow,predDeath,preyCarry,predCarry,mutate,outflow,velocity,t0);

      	}


    }


void Graph::initialiseLKSystems(double time, double dt)

    {
    /*
     * Written 29/9/17 by dh4gan
     * sets up all systems
     */

    for (int i=0; i<nVertices; i++)
	{
	vertices[i]->initialiseLKSystem(time,dt);
	vertices[i]->writeLKParametersToFile();
	}

    }

void Graph::updateLKSystems(double t)

    {
    /*
     * Written 29/9/17 by dh4gan
     * drives integration of LK equations for LKVertex objects
     */

    // Compute outward fluxes of all vertices
    for (int i=0; i<nVertices; i++)
	{

	vertices[i]->computeOutwardFlux(t);
	}

    // Now update system
    for (int i=0; i<nVertices; i++)
	{
	vertices[i]->updateLKSystem(t);
	}

    }

void Graph::writeLKSnapshots(double t)

{
  /*
   * Written 15/3/18 by dh4gan
   * Handles writing of data to file
   * for LKSystem objects in Graph
   *
   */

  for(int i=0; i<nVertices; i++)
    {
      vertices[i]->writePopulationsToFile(t);
    }

}



