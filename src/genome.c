/*
Copyright (C) 2016 Stephen Koren

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
#include "genome.h"
#include "neural.h"

//Constructor which takes full genome specs and puts them into the new one
genome_t* Genome_Init(int id, vector* traits, vector* neurons, vector* genes)
{
	genome_t* genome = malloc(sizeof(genome_t));
	if (genome == 0) return ((void*)1);

	genome->ID = id;
	genome->traits = traits;
	genome->neurons = neurons;
	genome->genes = genes;

	return genome;
}

//Constructor which takes in links (not genes) and creates a Genome
genome_t* Genome_Init_Links(int id, vector* traits, vector* neurons, vector* links)
{
	genome_t* genome = malloc(sizeof(genome_t));
	if (genome == 0) return ((void*)1);

	genome->ID = id;
	genome->traits = traits;
	genome->neurons = neurons;
	genome->genes = vector_init();

	//We go through the links and turn them into original genes
	for (int i = 0; i < links->count; i++) {
		nlink_t* curlink = vector_get(links, i);
		vector_add(genome->genes, Gene_Init_Trait(curlink->trait, curlink->weight, curlink->inode, curlink->onode, curlink->recurrent, 1.0, 0.0));
	}

	return genome;
}

// Copy constructor
genome_t* Genome_Init_Copy(genome_t* other)
{
	genome_t* genome = malloc(sizeof(genome_t));
	if (genome == 0) return ((void*)1);

	genome->ID = other->ID;
	genome->traits = vector_init();
	genome->neurons = vector_init();
	genome->genes = vector_init();

	for (int i = 0; i < other->traits->count; ++i) {
		vector_add(other->traits, Trait_Init_Copy(other->traits->data[i]));
	}

	trait_t *assoc_trait = 0;
	//Duplicate NNodes
	for (int i = 0; i < other->neurons->count; ++i) {
		neuron_t* curnode = (neuron_t*)other->neurons->data[i];
		if (curnode->trait == 0) assoc_trait = 0;
		else
		{
			for (int j = 0; j < other->traits->count; j++)
			{
				if (((trait_t*)other->traits->data[j])->id == curnode->trait_id)
				{
					assoc_trait = other->traits->data[j];
					break;
				}
			}
		}
		neuron_t* newnode = Neuron_Init_Derived(curnode, assoc_trait);
		curnode->dupe = newnode;
		vector_add(genome->neurons, newnode);
	}

	neuron_t *inode;
	neuron_t *onode;
	trait_t *trait;

	for (int i = 0; i < other->genes->count; ++i) {
		gene_t* curgene = (gene_t*)other->genes->data[i];
		inode = curgene->link->inode->dupe;
		onode = curgene->link->onode->dupe;
		trait = curgene->link->trait;

		if (trait == 0) assoc_trait = 0;
		else
		{
			for (int j = 0; j < other->traits->count; j++)
			{
				if (((trait_t*)other->traits->data[j])->id == trait->id)
				{
					assoc_trait = other->traits->data[j];
					break;
				}
			}
		}

		gene_t* newgene = Gene_Init_Dupe(curgene, assoc_trait, inode, onode);
		vector_add(genome->genes, newgene);
	}

	return genome;
}

//Special constructor which spawns off an input file
//This constructor assumes that some routine has already read in GENOMESTART
/* Genome_Init_File(int id, std::ifstream &iFile)
{
	char curword[128];  //max word size of 128 characters
	char curline[1024]; //max line size of 1024 characters
	char delimiters[] = " \n";

	int done = 0;

	//int pause;

	genome_id = id;

	iFile.getline(curline, sizeof(curline));
	int wordcount = NEAT::getUnitCount(curline, delimiters);
	int curwordnum = 0;

	//Loop until file is finished, parsing each line
	while (!done) {

		//std::cout << curline << std::endl;

		if (curwordnum > wordcount || wordcount == 0) {
			iFile.getline(curline, sizeof(curline));
			wordcount = NEAT::getUnitCount(curline, delimiters);
			curwordnum = 0;
		}

		std::stringstream ss(curline);
		//strcpy(curword, NEAT::getUnit(curline, curwordnum++, delimiters));
		ss >> curword;

		//printf(curword);
		//printf(" test\n");
		//Check for end of Genome
		if (strcmp(curword, "genomeend") == 0) {
			//strcpy(curword, NEAT::getUnit(curline, curwordnum++, delimiters));
			ss >> curword;
			int idcheck = atoi(curword);
			//iFile>>idcheck;
			if (idcheck != genome_id) printf("ERROR: id mismatch in genome");
			done = 1;
		}

		//Ignore genomestart if it hasn't been gobbled yet
		else if (strcmp(curword, "genomestart") == 0) {
			++curwordnum;
			//cout<<"genomestart"<<endl;
		}

		//Ignore comments surrounded by - they get printed to screen
		else if (strcmp(curword, "/*") == 0) {
			//strcpy(curword, NEAT::getUnit(curline, curwordnum++, delimiters));
			ss >> curword;
			while (strcmp(curword, "*\") != 0) {
				//cout<<curword<<" ";
				//strcpy(curword, NEAT::getUnit(curline, curwordnum++, delimiters));
				ss >> curword;
			}
			//cout<<endl;
		}

		//Read in a trait
		else if (strcmp(curword, "trait") == 0) {
			Trait *newtrait;

			char argline[1024];
			//strcpy(argline, NEAT::getUnits(curline, curwordnum, wordcount, delimiters));

			curwordnum = wordcount + 1;

			ss.getline(argline, 1024);
			//Allocate the new trait
			newtrait = new Trait(argline);

			//Add trait to vector of traits
			traits.push_back(newtrait);
		}

		//Read in a node
		else if (strcmp(curword, "node") == 0) {
			NNode *newnode;

			char argline[1024];
			//strcpy(argline, NEAT::getUnits(curline, curwordnum, wordcount, delimiters));
			curwordnum = wordcount + 1;

			ss.getline(argline, 1024);
			//Allocate the new node
			newnode = new NNode(argline, traits);

			//Add the node to the list of nodes
			nodes.push_back(newnode);
		}

		//Read in a Gene
		else if (strcmp(curword, "gene") == 0) {
			Gene *newgene;

			char argline[1024];
			//strcpy(argline, NEAT::getUnits(curline, curwordnum, wordcount, delimiters));
			curwordnum = wordcount + 1;

			ss.getline(argline, 1024);
			//std::cout << "New gene: " << ss.str() << std::endl;
			//Allocate the new Gene
			newgene = new Gene(argline, traits, nodes);

			//Add the gene to the genome
			genes.push_back(newgene);

			//std::cout<<"Added gene " << newgene << std::endl;
		}

	}
}*/

// This special constructor creates a Genome with random connectivity. 
// The last input is a bias.
// Linkprob is the probability of a link.
genome_t* Genome_Init_Structure(int new_id, int input_count, int output_count, int hidden_count, int hidden_max, cbool recurrent, double linkprob)
{
	genome_t* genome = malloc(sizeof(genome_t));
	if (genome == 0) return ((void*)1);

	genome->ID = new_id;
	genome->traits = vector_init();
	genome->neurons = vector_init();
	genome->genes = vector_init();

	int totalnodes = input_count + output_count + hidden_max;
	
	int matrixdim = totalnodes*totalnodes;
	int count;

	int ncount; //Node and connection counters

	int maxnode; //No nodes above this number for this genome

	int first_output; //Number of first output node

	cbool *cm = malloc(sizeof(cbool) * matrixdim);  //Dimension the connection matrix
	cbool *cmp; //Connection matrix pointer
	maxnode = input_count + hidden_count;

	first_output = totalnodes - output_count + 1;

	//For creating the new genes
	trait_t *newtrait;
	neuron_t *in_node;
	neuron_t *out_node;

	//cout<<"Assigned id "<<genome_id<<endl;

	//Step through the connection matrix, randomly assigning bits
	cmp = cm;
	for (count = 0; count < matrixdim; count++) {
		if (Random_Float() < linkprob)
			*cmp = true;
		else *cmp = false;
		cmp++;
	}

	//Create a dummy trait (this is for future expansion of the system)
	newtrait = Trait_Init_Values(10, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0);
	vector_add(genome->traits, newtrait);

	//Build the input nodes
	for (int i = 1; i <= i; i++)
	{
		neuron_t *newnode = Neuron_Init_Placement(NQ_SENSOR, i, (i < input_count) ? NQ_INPUT : NQ_BIAS);
		newnode->trait = newtrait;

		//Add the node to the list of nodes
		vector_add(genome->neurons, newnode);
	}

	//Build the hidden nodes
	for (int i = 1; i <= i; i++)
	{
		neuron_t *newnode = Neuron_Init_Placement(NQ_NEURON, i, NQ_HIDDEN);
		newnode->trait = newtrait;
		vector_add(genome->neurons, newnode);
	}

	//Build the output nodes
	for (int i = 1; i <= i; i++)
	{
		neuron_t *newnode = Neuron_Init_Placement(NQ_NEURON, i, NQ_OUTPUT);
		newnode->trait = newtrait;
		vector_add(genome->neurons, newnode);
	}

	//Step through the connection matrix, creating connection genes
	cmp = cm;
	count = 0;
	for (int col = 1; col <= totalnodes; col++)
	{
		for (int row = 1; row <= totalnodes; row++)
		{
			//Only try to create a link if it is in the matrix and not leading into a sensor.
			if ((*cmp == true) && (col > input_count) &&
				((col <= maxnode) || (col >= first_output)) &&
				((row <= maxnode) || (row >= first_output))) {
				//If it isn't recurrent, create the connection no matter what
				if (col > row) {

					//Find the input node.
					int node_iter = 0;
					for (; node_iter < totalnodes; node_iter++)
					{
						neuron_t* cur_node = (neuron_t*)genome->neurons->data[node_iter];
						if (cur_node->node_id == row) {
							in_node = cur_node;
							break;
						}
					}
					//Find the output node. No need to reset node_iter; output node will always be after the input.
					for (; node_iter < totalnodes; node_iter++)
					{
						neuron_t* cur_node = (neuron_t*)genome->neurons->data[node_iter];
						if (cur_node->node_id == col) {
							out_node = cur_node;
							break;
						}
					}

					//Create the gene
					double new_weight = Random_Sign() * Random_Float();

					//Add the gene to the genome
					vector_add(genome->genes, Gene_Init_Trait(newtrait, new_weight, in_node, out_node, false, count, new_weight));
				}
				else if (recurrent) { //Create a recurrent connection

					//Retrieve the in_node
					int node_iter = 0;
					for (; node_iter < totalnodes; node_iter++)
					{
						neuron_t* cur_node = (neuron_t*)genome->neurons->data[node_iter];
						if (cur_node->node_id == row) {
							in_node = cur_node;
							break;
						}
					}
					//Reset node_iter; this time we're not sure if it's before or after.
					for (node_iter = 0; node_iter < totalnodes; node_iter++)
					{
						neuron_t* cur_node = (neuron_t*)genome->neurons->data[node_iter];
						if (cur_node->node_id == col) {
							out_node = cur_node;
							break;
						}
					}

					//Create the gene
					double new_weight = Random_Sign() * Random_Float();

					//Add the gene to the genome
					vector_add(genome->genes, Gene_Init_Trait(newtrait, new_weight, in_node, out_node, false, count, new_weight));
				}
			}
			count++; //increment gene counter	    
			cmp++;
		}
	}
	free(cm);
	return genome;
}

//Special constructor that creates a Genome of 3 possible types:
//0 - Fully linked, no hidden nodes
//1 - Fully linked, one hidden node splitting each link
//2 - Fully connected with a hidden layer, recurrent 
//num_hidden is only used in type 2
genome_t* Genome_Init_Auto(int num_in, int num_out, int num_hidden, int type)
{
	genome_t* genome = malloc(sizeof(genome_t));
	if (genome == 0) return ((void*)1);

	genome->ID = 0;
	genome->traits = vector_init();
	genome->neurons = vector_init();
	genome->genes = vector_init();

	//Temporary lists of nodes
	vector *inputs = vector_init();
	vector *outputs = vector_init();
	vector *hidden = vector_init();
	neuron_t *bias; //Remember the bias

	//For creating the new genes
	neuron_t *newnode;
	gene_t *newgene;
	trait_t *newtrait;

	int count;
	
	//Create a dummy trait (this is for future expansion of the system)
	newtrait = Trait_Init_Values(1, 0, 0, 0, 0, 0, 0, 0, 0, 0);
	vector_add(genome->traits, newtrait);

	//Adjust hidden number
	if (type == 0)
		num_hidden = 0;
	else if (type == 1)
		num_hidden = num_in*num_out;

	//Create the inputs and outputs

	//Build the input nodes
	for (int i = 1; i <= num_in; i++) {
		if (i < num_in)
		{
			newnode = Neuron_Init_Placement(NQ_SENSOR, i, NQ_INPUT);
		}
		else
		{
			newnode = Neuron_Init_Placement(NQ_SENSOR, i, NQ_BIAS);
			bias = newnode;
		}

		//newnode->nodetrait=newtrait;

		//Add the node to the list of nodes
		vector_add(genome->neurons, newnode);
		vector_add(inputs, newnode);
	}

	//Build the hidden nodes
	for (int i = num_in + 1; i <= num_in + num_hidden; i++) {
		newnode = Neuron_Init_Placement(NQ_NEURON, i, NQ_HIDDEN);
		//newnode->nodetrait=newtrait;
		//Add the node to the list of nodes
		vector_add(genome->neurons, newnode);
		vector_add(hidden, newnode);
	}

	//Build the output nodes
	for (int i = num_in + num_hidden + 1; i <= num_in + num_hidden + num_out; i++) {
		newnode = Neuron_Init_Placement(NQ_NEURON, i, NQ_OUTPUT);
		//newnode->nodetrait=newtrait;
		//Add the node to the list of nodes
		vector_add(genome->neurons, newnode);
		vector_add(outputs, newnode);
	}

	//Create the links depending on the type
	if (type == 0) {
		//Just connect inputs straight to outputs

		count = 1;

		//Initialize inputs and outputs.
		for (int i = 0; i < outputs->count; i++)
		{
			for (int j = 0; j < inputs->count; j++)
			{
				//Connect each input to each output
				vector_add(genome->genes, Gene_Init_Trait(newtrait, 0, inputs->data[j], outputs->data[i], false, count, 0));

				count++;
			}
		}
	} //end type 0
	//A split link from each input to each output
	else if (type == 1) {
		count = 1;

		//Initialize links between layers.
		for (int i = 0; i < outputs->count; i++)
		{
			for (int j = 0; j < inputs->count; j++)
			{
				for (int k = 0; k < hidden->count; k++)
				{
					// Connect Input to hidden.
					vector_add(genome->genes, Gene_Init_Trait(newtrait, 0, inputs->data[j], hidden->data[k], false, count, 0));
					count++;

					// Connect hidden to output.
					vector_add(genome->genes, Gene_Init_Trait(newtrait, 0, hidden->data[k], outputs->data[i], false, count, 0));
					count++;
				}
			}
		}

	}//end type 1
	//Fully connected 
	else if (type == 2) {
		count = 1; //Start gene counter at 1


		//Connect all inputs to all hidden nodes
		for (int i = 0; i < hidden->count; i++)
		{
			for (int j = 0; j < inputs->count; j++)
			{
				// Connect each input to each hidden.
				vector_add(genome->genes, Gene_Init_Trait(newtrait, 0, inputs->data[j], hidden->data[i], false, count, 0));
				count++;
			}
		}
		
		//Connect all hidden units to all outputs
		for (int i = 0; i < outputs->count; i++)
		{
			for (int j = 0; j < hidden->count; j++)
			{
				vector_add(genome->genes, Gene_Init_Trait(newtrait, 0, hidden->data[j], outputs->data[i], false, count, 0));
				count++;
			}
		}

		//Connect the bias to all outputs
		for (int i = 0; i < outputs->count; i++)
		{
			vector_add(genome->genes, Gene_Init_Trait(newtrait, 0, bias, outputs->data[i], false, count, 0));
			count++;
		}

		//Recurrently connect the hidden nodes
		for (int i = 0; i < hidden->count; i++)
		{
			for (int j = 0; j < hidden->count; j++)
			{
				vector_add(genome->genes, Gene_Init_Trait(newtrait, 0, hidden->data[j], hidden->data[i], true, count, 0));
				count++;
			}
		}

	}//end type 2

	return genome;
}

// Loads a new Genome from a file (doesn't require knowledge of Genome's id)
static genome_t *new_Genome_load(char *filename);

//Destructor kills off all lists (including the trait vector)
void Genome_Delete(genome_t *genome)
{
	for (int i = 0; i < vector_count(genome->traits); i++) {
		Trait_Delete(vector_get(genome->traits, i));
	}

	free(genome);
}

//Generate a network phenotype from this Genome with specified id
network_t *Genome_Genesis(genome_t *genome, int id)
{
	neuron_t *newnode;
	trait_t *curtrait;
	nlink_t *curlink;
	nlink_t *newlink;

	double maxweight = 0.0; //Compute the maximum weight for adaptation purposes
	double weight_mag; //Measures absolute value of weights

	//Inputs and outputs will be collected here for the network
	//All nodes are collected in an all_list- 
	//this will be used for later safe destruction of the net
	vector* inlist = vector_init();
	vector* outlist = vector_init();
	vector* all_list = vector_init();

	//Gene translation variables
	neuron_t *inode;
	neuron_t *onode;

	//Create the nodes
	for (int i = 0; i < genome->neurons->count; ++i) {
		neuron_t* curnode = (neuron_t*)genome->neurons->data[i];

		newnode = Neuron_Init(curnode->type, curnode->node_id);

		//Derive the node parameters from the trait pointed to
		Neuron_Derive_Trait(curnode, curnode->trait);

		//Check for input or output designation of node
		if (curnode->node_label == NQ_INPUT)  vector_add(inlist, newnode);
		if (curnode->node_label == NQ_BIAS)   vector_add(inlist, newnode);
		if (curnode->node_label == NQ_OUTPUT) vector_add(outlist, newnode);

		//Keep track of all nodes, not just input and output
		vector_add(all_list, newnode);

		//Have the node specifier point to the node it generated
		curnode->analogue = newnode;
	}

	//Create the links by iterating through the genes
	for (int i = 0; i < genome->genes->count; ++i) {
		gene_t* curgene = (gene_t*)genome->genes->data[i];
		//Only create the link if the gene is enabled
		if (curgene->enabled == true) {
			curlink = curgene->link;
			inode = curlink->inode->analogue;
			onode = curlink->onode->analogue;
			//NOTE: This line could be run through a recurrency check if desired
			// (no need to in the current implementation of NEAT)
			newlink = Link_Init(curlink->weight, inode, onode, curlink->recurrent);

			vector_add(onode->links_in, newlink);
			vector_add(inode->links_out, newlink);

			//Derive link's parameters from its Trait pointer
			curtrait = curlink->trait;

			Link_Derive_Trait(newlink, curtrait);

			//Keep track of maximum weight
			if (newlink->weight>0)
				weight_mag = newlink->weight;
			else weight_mag = -newlink->weight;
			if (weight_mag>maxweight)
				maxweight = weight_mag;
		}
	}

	//Create the new network
	network_t *newnet = Network_Init(inlist, outlist, all_list, id);

	//Attach genotype and phenotype together
	newnet->genotype = genome;
	genome->phenotype = newnet;

	newnet->maxweight = maxweight;

	return newnet;
}

genome_t *Genome_Duplicate(genome_t *genome, int new_id)
{
	//Collections for the new Genome
	vector *traits = vector_init(), *nodes = vector_init(), *genes = vector_init();

	//Trait associated with current item
	trait_t *assoc_trait;
	
	//Duplicate the traits
	for (int i = 0; i < genome->traits->count; i++)
		vector_add(traits, Trait_Init_Copy(genome->traits->data[i]));

	//Duplicate NNodes
	for (int i = 0; i < genome->neurons->count; i++)
	{
		neuron_t *curnode = genome->neurons->data[i];
		//First, find the trait that this node points to
		if (curnode->trait == 0) assoc_trait = 0;
		else 
			for (int j = 0; j < traits->count; j++) 
				if (((trait_t*)traits->data[j])->id == curnode->trait->id)
					assoc_trait = traits->data[j];

		neuron_t *newnode = Neuron_Init_Derived(curnode, assoc_trait);

		curnode->dupe = newnode;  //Remember this node's old copy
		vector_add(nodes, newnode);
	}

	//Duplicate Genes
	for (int i = 0; i < genome->genes->count; i++)
	{
		gene_t *curgene = genome->genes->data[i];

		//First find the nodes connected by the gene's link
		neuron_t *inode = curgene->link->inode->dupe;
		neuron_t *onode = curgene->link->onode->dupe;

		//Get a pointer to the trait expressed by this gene
		trait_t *traitptr = curgene->link->trait;

		if (traitptr == 0) assoc_trait = 0;
		else
			for (int j = 0; j < traits->count; j++)
				if (((trait_t*)traits->data[j])->id == traitptr->id)
					assoc_trait = traits->data[j];

		vector_add(genes, Gene_Init_Dupe(curgene, assoc_trait, inode, onode));
	}

	//Finally, return the genome
	return Genome_Init(new_id, traits, nodes, genes);
}

cbool Genome_Verify(genome_t *genome)
{
	// Check all gene nodes.
	for (int i = 0; i < genome->genes->count; i++) 
	{
		neuron_t *inode = ((gene_t*)genome->genes->data[i])->link->inode;
		neuron_t *onode = ((gene_t*)genome->genes->data[i])->link->onode;
		
		neuron_t *cur_inode = 0, *cur_onode = 0;
		for (int j = 0; j < genome->neurons->count; j++)
		{
			if (cur_inode != inode) cur_inode = genome->neurons->data[j];
			if (cur_onode != onode) cur_onode = genome->neurons->data[j];
			if (cur_onode == onode && cur_inode == inode) break;
		}

		if (cur_inode == genome->neurons->data[genome->neurons->count-1] || 
			cur_onode == genome->neurons->data[genome->neurons->count-1]) 
			return false;
	}

	int last_id = 0;

	// Ensure no node is out of order.
	for (int j = 0; j < genome->neurons->count; j++)
	{
		neuron_t *curnode = (neuron_t*)genome->neurons->data[j];
		if (curnode->node_id < last_id) return false;
		last_id = curnode->node_id;
	}

	return true;
}

int Genome_Get_Last_Node_ID(genome_t *genome)
{
	return ((neuron_t*)genome->neurons->data[genome->neurons->count - 1])->node_id + 1;
}

double Genome_Get_Last_Gene_Innovnum(genome_t *genome)
{
	return ((gene_t*)genome->genes->data[genome->genes->count - 1])->innovation_num + 1;
}

void Genome_Print_Genome(genome_t *genome)
{

}

void Genome_Mutate_Random_Trait(genome_t *genome)
{
	Trait_Mutate((trait_t*)genome->traits->data[Random_Int(0, genome->traits->count)]);
}

void Genome_Mutate_Link_Trait(genome_t *genome, int times)
{
	if (genome->genes->count == 0) return;

	for (int i = 0; i < times; i++) {
		//Choose a random gene.
		gene_t* gene = genome->genes->data[Random_Int(0, genome->genes->count - 1)];

		//Do not alter frozen genes
		if (!gene->frozen)
			gene->link->trait = genome->traits->data[Random_Int(0, genome->traits->count - 1)];
	}
}

void Genome_Mutate_Node_Trait(genome_t *genome, int times)
{
	if (genome->neurons->count == 0) return;

	for (int i = 0; i < times; i++) {
		//Choose a random node.
		neuron_t* node = genome->neurons->data[Random_Int(0, genome->neurons->count - 1)];

		//Do not alter frozen genes
		if (!node->frozen)
			node->trait = genome->traits->data[Random_Int(0, genome->traits->count - 1)];
	}
}

void Genome_Mutate_Link_Weights(genome_t *genome, double power, double rate, enum mutator_e mut_type)
{	
	//The power of mutation will rise farther into the genome
	//on the theory that the older genes are more fit since
	//they have stood the test of time

	double num = 0.0; //counts gene placement
	double endpart = genome->genes->count*0.8;; //Signifies the last part of the genome

	//Go through all the Genes and perturb their link's weights
	for (int i = 0; i < genome->genes->count; i++) {
		gene_t *curgene = genome->genes->data[i];
		//Don't mutate weights of frozen links
		if (!curgene->frozen) {
			double gausspoint = 1.0 - rate;
			double coldgausspoint = 1.0 - rate;

			//Once in a while really shake things up
			if (Random_Float() > 0.5)
			{
				gausspoint = 0.3;
				coldgausspoint = 0.1;
			}
			else if ((genome->genes->count >= 10) && (num > endpart))
			{
				gausspoint = 0.5;  //Mutate by modification % of connections
				coldgausspoint = 0.3; //Mutate the rest by replacement % of the time
			}
			else if (Random_Float() > 0.5)
			{
				coldgausspoint -= 0.1;
			}

			//Possible methods of setting the perturbation:
			//randnum=gaussrand()*powermod;
			//randnum=gaussrand();

			double randnum = Random_Sign() * Random_Float() * power;
			//std::cout << "RANDOM: " << randnum << " " << randposneg() << " " << randfloat() << " " << power << " " << powermod << std::endl;
			if (mut_type == NQ_GAUSSIAN) {
				double randchoice = Random_Float();
				if (randchoice > gausspoint)
					curgene->link->weight += randnum;
				else if (randchoice > coldgausspoint)
					curgene->link->weight = randnum;
			}
			else if (mut_type == NQ_COLDGAUSSIAN)
				curgene->link->weight = randnum;

			//Cap the weights at 8.0 (experimental)
			if (curgene->link->weight > 8.0) curgene->link->weight = 8.0;
			else if (curgene->link->weight < -8.0) curgene->link->weight = -8.0;

			//Record the innovation
			//(*curgene)->mutation_num+=randnum;
			curgene->mutation_num = curgene->link->weight;

			num += 1.0;

		}

	}
}

// toggle genes on or off 
void Genome_Mutate_Toggle_Enable(genome_t *genome, int times)
{
	// Mutate a number of times.
	for (int i = 0; i < times; i++) {
		//Choose a random gene
		gene_t* gene = genome->genes->data[Random_Int(0, genome->genes->count - 1)];

		//Toggle the enable on this gene
		if (gene->enabled) {

			//We need to make sure that another gene connects out of the in-node
			//Because if not a section of network will break off and become isolated
			for (int j = 0; j < genome->genes->count; j++)
			{
				gene_t* other = genome->genes->data[j];
				if (other->link->inode == gene->link->inode &&
					other->enabled &&
					other->innovation_num != gene->innovation_num)
				{
					gene->enabled = false;
					break;
				}	
			}
		}
		else gene->enabled = true;
	}
}

// Find first disabled gene and enable it 
void Genome_Mutate_Gene_Reenable(genome_t *genome)
{
	for (int i = 0; i < genome->genes->count; i++)
	{
		gene_t *gene = genome->genes->data[i];
		if (!gene->enabled)
		{
			gene->enabled = true;
			break;
		}
	}
}

// These last kinds of mutations return false if they fail
//   They can fail under certain conditions,  being unable
//   to find a suitable place to make the mutation.
//   Generally, if they fail, they can be called again if desired. 

cbool Genome_Mutate_Add_Node(genome_t *genome, vector *innovs, int curnode_id, double curinnov)
{
	//double randmult;  //using a gaussian to find the random gene

	cbool found = false;
	gene_t *gene = 0;

	// Gene selection which skews towards older genes, whilst still retaining some randomness.
	if (true) {
		for (int i = 0; i < genome->genes->count; i++) 
		{
			gene = genome->genes->data[i];
			if (gene->enabled && gene->link->inode->node_label != NQ_BIAS)
			{
				for (int j = i; j < genome->genes->count; j++)
				{
					gene = genome->genes->data[j];
					// Add some randomisation in, with a higher chance of getting older nodes.
					// This encourages splitting to distribute evenly.
					if (Random_Float() > 0.3 && gene->link->inode->node_label != NQ_BIAS)
					{
						if (!gene->enabled) return false;
						found = true;
						break;
					}
				}
				break;
			}
		}
	}
	// Obsolated totally random gene selection method.
	else 
	{
		for (int i = 0; (i < 20 && !found); i++) 
		{
			gene = genome->genes->data[Random_Int(0, genome->genes->count - 1)];
			if (gene->enabled && gene->link->inode->node_label != NQ_BIAS)
				found = true;
		}
	}

	//If we couldn't find anything say goodbye
	if (!found) return false;

	//Disabled the gene
	gene->enabled = false;

	//Extract the link
	nlink_t *link = gene->link;

	//The weight of the original link
	double oldweight = gene->link->weight;

	//Extract the nodes
	neuron_t *inode = link->inode;
	neuron_t *onode = link->onode;

	//The new Genes
	gene_t *newgene1 = 0;
	gene_t *newgene2 = 0;

	//The new node
	neuron_t *newnode = 0;

	//Check to see if this innovation has already been done   
	//in another genome
	//Innovations are used to make sure the same innovation in
	//two separate genomes in the same generation receives
	//the same innovation number.
	for (int i = 0; i <= innovs->count; i++) 
	{
		if (i == innovs->count) // We couldn't find a related innovation. Make a totally novel one!
		{
			//Get the old link's trait
			trait_t *traitptr = link->trait;

			//Create the new NNode
			//By convention, it will point to the first trait
			newnode = Neuron_Init_Placement(NQ_NEURON, curnode_id++, NQ_HIDDEN);
			newnode->trait = genome->traits->data[0];

			//Create the new Genes
			newgene1 = Gene_Init_Trait(traitptr, 1.0, inode, newnode, (link->recurrent), curinnov, 0);
			newgene2 = Gene_Init_Trait(traitptr, oldweight, newnode, onode, false, curinnov + 1, 0);
			curinnov += 2.0;

			//Add the innovations (remember what was done)
			vector_add(innovs, Innovation_Init(inode->node_id, onode->node_id, curinnov - 2.0, curinnov - 1.0, newnode->node_id, gene->innovation_num));
			break;
		}
		else 
		{
			innovation_t* innovation = innovs->data[i];
			if (innovation->innovation_type == NQ_NEWNODE &&
				innovation->node_in_id == inode->node_id &&
				innovation->node_out_id == onode->node_id &&
				innovation->old_innov_num == gene->innovation_num)
			{
				//Here, the innovation has been done before

				//Get the old link's trait
				trait_t *traitptr = link->trait;

				//Create the new NNode
				newnode = Neuron_Init_Placement(NQ_NEURON, innovation->new_node_id, NQ_HIDDEN);
				//By convention, it will point to the first trait
				//Note: In future may want to change this
				newnode->trait = genome->traits->data[0];

				//Create the new Genes
				newgene1 = Gene_Init_Trait(traitptr, 1.0, inode, newnode, (link->recurrent), innovation->innovation_num1, 0);
				newgene2 = Gene_Init_Trait(traitptr, oldweight, newnode, onode, false, innovation->innovation_num2, 0);
				break;
			}
		}
	}

	//Now add the new NNode and new Genes to the Genome
	//genes.push_back(newgene1);   //Old way to add genes- may result in genes becoming out of order
	//genes.push_back(newgene2);
	Genome_Add_Gene(genome, genome->genes, newgene1);  //Add genes in correct order
	Genome_Add_Gene(genome, genome->genes, newgene2);
	Genome_Node_Insert(genome, genome->neurons, newnode);

	return true;
}

cbool Genome_Mutate_Add_Link(genome_t *genome, vector *innovs, double curinnov, int tries)
{
	cbool found = false;  //Tells whether an open pair was found
	cbool recurflag; //Indicates whether proposed link is recurrent

	//These are used to avoid getting stuck in an infinite loop checking
	//for recursion
	//Note that we check for recursion to control the frequency of
	//adding recurrent links rather than to prevent any paricular
	//kind of error
	int thresh = (genome->neurons->count) * (genome->neurons->count);

	//Make attempts to find an unconnected pair
	int trycount = 0; //Iterates over attempts to find an unconnected pair of nodes

	//Decide whether to make this recurrent
	cbool do_recur = (Random_Float() < NQ_RECUR_ONLY_CHANCE);

	//Find the first non-sensor so that the to-node won't look at sensors as
	//possible destinations
	int first_nonsensor = 0;

	//Get the index of the first non-sensor node.
	for (int first_nonsensor = 0; first_nonsensor < genome->neurons->count; first_nonsensor++)
		if (((neuron_t*)genome->neurons->data[first_nonsensor])->type != NQ_SENSOR)
			break;

	neuron_t *nodep1 = 0, *nodep2 = 0; //Pointers to the nodes

	//Here is the recurrent finder loop- it is done separately

	//Loop to find a nonrecurrent link
	for (int i = 0; i < tries; i++) 
	{
		//Choose random nodenums

		int nodenum1, nodenum2;

		//Some of the time try to make a recur loop
		if (do_recur && (Random_Float() > 0.5))
		{
			nodenum1 = Random_Int(first_nonsensor, genome->neurons->count - 1);
			nodenum2 = nodenum1;
		}
		else
		{
			nodenum1 = Random_Int(0, genome->neurons->count - 1);
			nodenum2 = Random_Int(first_nonsensor, genome->neurons->count - 1);
		}

		nodep1 = genome->genes->data[nodenum1];
		nodep2 = genome->genes->data[nodenum2];

		//Storage for an existing link.
		gene_t *gene = 0;

		//See if a link already exists  ALSO STOP AT END OF GENES!!!!
		for (int j = 0; j < genome->genes->count; j++)
		{
			if (nodep2->type == NQ_SENSOR &&
				((gene_t*)genome->genes->data[j])->link->inode == nodep1 &&
				((gene_t*)genome->genes->data[j])->link->onode == nodep2 &&
				((gene_t*)genome->genes->data[j])->link->recurrent == do_recur)
			{
				gene = genome->genes->data[j];
				break;
			}
		}

		if (gene == 0)
		{
			recurflag = Network_Is_Recur(genome->phenotype, nodep1->analogue, nodep2->analogue, 0, thresh);

			//ADDED: CONSIDER connections out of outputs recurrent
			if (nodep1->type == NQ_OUTPUT || nodep2->type == NQ_OUTPUT)
				recurflag = true;

			//Make sure it finds the right kind of link (recur or not)
			if (!(do_recur && recurflag) || recurflag )
			{
				found = true;
				break;
			}

		}

	} //End of normal link finding loop

	//Continue only if an open link was found
	if (found) {
		//If it was supposed to be recurrent, make sure it gets labeled that way
		if (do_recur) recurflag = true;

		gene_t *newgene = 0;  //The new Gene
		for (int i = 0; i <= innovs->count; i++)
		{
			if (i == innovs->count) // We couldn't find a related innovation. Make a totally novel one!
			{
				//If the phenotype does not exist, exit on false,print error
				//Note: This should never happen- if it does there is a bug
				if (genome->phenotype == 0) return false;

				//Choose a random trait
				int trait_num = Random_Int(0, genome->traits->count - 1);

				//Choose the new weight
				//newweight=(gaussrand())/1.5;  //Could use a gaussian
				double newweight = Random_Sign() * Random_Float();

				newgene = Gene_Init_Trait(genome->traits->data[trait_num], newweight, nodep1, nodep2, recurflag, curinnov, newweight);

				//Add the innovation
				vector_add(innovs, Innovation_Init_Link(nodep1->node_id, nodep2->node_id, curinnov, newweight, trait_num));

				curinnov = curinnov + 1.0;
			}
			else 
			{
				innovation_t* innovation = innovs->data[i];
				if (innovation->innovation_type == NQ_NEWLINK &&
					innovation->node_in_id == nodep1->node_id &&
					innovation->node_out_id == nodep2->node_id &&
					innovation->recur_flag == recurflag)
				{
					//Here, the innovation has been done before

					//Create new gene
					newgene = Gene_Init_Trait(genome->traits->data[innovation->new_trait_num], innovation->new_weight, nodep1, nodep2, recurflag, innovation->innovation_num1, 0);
					break;
				}
			}
		}
		Genome_Add_Gene(genome, genome->genes, newgene); //Adds the gene in correct order
		return true;
	}

	return false;
}

void Genome_Mutate_Add_Sensor(genome_t *genome, vector *innovs, double curinnov)
{
	vector *sensors = vector_init(), *outputs = vector_init();

	//Find all the sensors and outputs
	for (int i = 0; i < genome->neurons->count; i++) 
	{
		neuron_t *node = genome->neurons->data[i];

		if ((node->type) == NQ_SENSOR)
			vector_add(sensors, node);
		else if (node->node_label == NQ_OUTPUT)
			vector_add(outputs, node);
	}

	for (int i = 0; i < sensors->count; i++)
	{
		neuron_t *sensor = sensors->data[i];

		int outputConnections = 0;

		for (int j = 0; j < genome->genes->count; j++)
			if (((gene_t*)genome->genes->data[j])->link->onode->node_label == NQ_OUTPUT)
				outputConnections++;


		if (outputConnections == outputs->count) {
			vector_delete(sensors, i); //Does this work? remove by number from a vector?
		}
	}

	//If all sensors are connected, quit
	if (sensors->count == 0) return;

	//Pick randomly from remaining sensors
	neuron_t *sensor = sensors->data[Random_Int(0, sensors->count - 1)];

	//Add new links to chosen sensor, avoiding redundancy


	for (int i = 0; i < outputs->count; i++)
	{
		neuron_t *output = outputs->data[i];
		cbool found = false;

		for (int j = 0; j < genome->genes->count; j++) 
		{
			gene_t *gene = genome->genes->data[j];
			if (gene->link->inode == sensor && gene->link->onode == output) 
				found = true;
		}

		//Record the innovation
		if (!found) {
			gene_t* newgene = 0;
			for (int j = 0; j <= innovs->count; j++)
			{
				if (j == innovs->count) // We couldn't find a related innovation. Make a totally novel one!
				{

					//Choose a random trait
					int trait_num = Random_Int(0, genome->traits->count - 1);

					//Choose the new weight
					//newweight=(gaussrand())/1.5;  //Could use a gaussian
					double newweight = Random_Sign() * Random_Float() * 3.0; //used to be 10.0

					//Create the new gene
					newgene = Gene_Init_Trait(genome->traits->data[trait_num], newweight, sensor, output, false, curinnov, newweight);

					//Add the innovation
					vector_add(innovs, Innovation_Init_Link(sensor->node_id, output->node_id, curinnov, newweight, trait_num));

					curinnov = curinnov + 1.0;
				}
				else
				{
					innovation_t* innovation = innovs->data[i];
					if (innovation->innovation_type == NQ_NEWLINK &&
						innovation->node_in_id == sensor->node_id &&
						innovation->node_out_id == output->node_id &&
						innovation->recur_flag == false)
					{
						//Create new gene
						newgene = Gene_Init_Trait(genome->traits->data[innovation->new_trait_num], innovation->new_weight, sensor, output, false, innovation->innovation_num1, 0);
						break;
					}
				}
			}
			Genome_Add_Gene(genome, genome->genes, newgene);  //adds the gene in correct order
		} 
	}
}

genome_t *Genome_Mate_Multipoint(genome_t *genome, genome_t *other, int genomeid, double fitness1, double fitness2, cbool interspec_flag)
{
	//The baby Genome will contain these new Traits, NNodes, and Genes
	vector *newtraits = vector_init(), *newnodes = vector_init(), *newgenes = vector_init();

	//iterators for moving through the two parents' genes
	neuron_t *inode;  //NNodes connected to the chosen Gene
	neuron_t *onode;
	int nodetraitnum;  //Trait number for a NNode

	//First, average the Traits from the 2 parents to form the baby's Traits
	//It is assumed that trait lists are the same length
	//In the future, may decide on a different method for trait mating
	for (int i = 0; i < genome->traits->count; i++)
		vector_add(newtraits, Trait_Init_Merge(genome->traits->data[i], other->traits->data[i]));

	//Figure out which genome is better
	//The worse genome should not be allowed to add extra structural baggage
	//If they are the same, use the smaller one's disjoint and excess genes only
	cbool p1better = (fitness1 > fitness2) ? true : (fitness1 == fitness2) ? (genome->genes->count < other->genes->count) : false;

	//NEW 3/17/03 Make sure all sensors and outputs are included
	for (int i = 0; i < other->neurons->count; i++)
	{
		neuron_t *curnode = other->neurons->data[i];
		if (curnode->node_label == NQ_INPUT ||
			curnode->node_label == NQ_OUTPUT ||
			curnode->node_label == NQ_BIAS)
		{
			nodetraitnum = (curnode->trait) ? (curnode->trait->id - ((trait_t*)genome->traits->data[0])->id) : 0;
			Genome_Node_Insert(genome, newnodes, Neuron_Init_Derived(curnode, newtraits->data[nodetraitnum]));
		}
	}


	//Now move through the Genes of each parent until both genomes end
	int loop_length = max(genome->genes->count, other->genes->count);
	for (int i = 0; i < loop_length; i++)
	{
		// Should we skip the excess genes in the larger of the two genomes?
		cbool skip = false;
		cbool disable = false; //Set to true if we want to disabled a chosen gene
		gene_t *chosengene = 0;  //Gene chosen for baby to inherit

		// Pick a gene to pass on to the child.
		if (i >= genome->genes->count) // We've run out of genes from out first genome. Use the second gene if it's better.
		{
			chosengene = other->genes->data[i];
			if (p1better) skip = true;  //Skip excess from the worse genome.
		}
		else if (i >= other->genes->count) // Vice versa
		{
			chosengene = genome->genes->data[i];
			if (!p1better) skip = true;  //Skip excess from the worse genome.
		}
		else
		{
			gene_t* p1gene = genome->genes->data[i];
			gene_t* p2gene = other->genes->data[i];

			double p1innov = p1gene->innovation_num;
			double p2innov = p2gene->innovation_num;

			if (p2innov == p1innov)
			{
				//Both innovations are the same; randomly pick one.
				chosengene = (Random_Float() < 0.5) ? p1gene : p2gene;

				//If one of the genes is disabled, the new one will likely also be.
				if ((!p1gene->enabled || !p2gene->enabled) && Random_Float(0.75)) disable = true;
			}
			else if (p1innov < p2innov)
			{
				chosengene = p1gene;
				if (!p1better) skip = true;
			}
			else if (p2innov < p1innov)
			{
				chosengene = p2gene;
				if (p1better) skip = true;
			}
		}
		/*
		//Uncomment this line to let growth go faster (from both parents excesses)
		skip=false;

		//For interspecies mating, allow all genes through:
		if (interspec_flag)
		skip=false;
		*/


		//Check to see if the chosengene conflicts with an already chosen gene
		//i.e. do they represent the same link    

		gene_t* checkedgene = 0;
		for (int j = 0; j < newgenes->count; j++) {
			//Check if they either share the same node IDs, or if their inputs and outputs are recursive and link between each other.
			if ((checkedgene->link->inode->node_id == chosengene->link->inode->node_id &&
				checkedgene->link->onode->node_id == chosengene->link->onode->node_id &&
				checkedgene->link->recurrent == chosengene->link->recurrent) ||
				(checkedgene->link->inode->node_id == chosengene->link->onode->node_id &&
				checkedgene->link->onode->node_id == chosengene->link->inode->node_id &&
				checkedgene->link->recurrent && chosengene->link->recurrent))
			{
				skip = true;
				break;
			}
		}


		//Now add the chosengene to the baby
		if (!skip)
		{
			neuron_t *new_inode = 0;
			neuron_t *new_onode = 0;

			int trait_num = 0;
			//Get the trait pointer.
			if (chosengene->link->trait == 0)
				trait_num = ((trait_t*)genome->traits->data[0])->id - 1;
			else //The subtracted number normalizes depending on whether traits start counting at 1 or 0
				trait_num = chosengene->link->trait->id - ((trait_t*)genome->traits->data[0])->id;

			//Next check for the nodes, add them if not in the baby Genome already

			//Swap the inode and onode depending on which will come first in the genome.s
			if (chosengene->link->inode->node_id < chosengene->link->onode->node_id)
			{
				inode = chosengene->link->inode;
				onode = chosengene->link->onode;
			}
			else
			{
				onode = chosengene->link->inode;
				inode = chosengene->link->onode;
			}
			//Checking for inode's existence
			neuron_t* curnode = 0;
			for (int j = 0; j < newnodes->count; j++)
			{
				if (((neuron_t*)newnodes->data[j])->node_id == inode->node_id)
				{
					curnode = newnodes->data[j];
					break;
				}
			}

			if (curnode == 0) 
			{
				nodetraitnum = (inode->trait) ? inode->trait->id - (((trait_t*)genome->traits->data[0])->id) : 0;
				Genome_Node_Insert(genome, newnodes, Neuron_Init_Derived(inode, newtraits->data[nodetraitnum]));
			}
			else new_inode = curnode;

			//Getting onode this time.
			for (int j = 0; j < newnodes->count; j++)
			{
				if (((neuron_t*)newnodes->data[j])->node_id == onode->node_id)
				{
					curnode = newnodes->data[j];
					break;
				}
			}

			if (curnode == 0)
			{
				nodetraitnum = (onode->trait) ? onode->trait->id - (((trait_t*)genome->traits->data[0])->id) : 0;
				Genome_Node_Insert(genome, newnodes, Neuron_Init_Derived(onode, newtraits->data[nodetraitnum]));
			}
			else new_onode = curnode;


			//Add the Gene
			gene_t* newgene = Gene_Init_Dupe(chosengene, newtraits->data[trait_num], new_inode, new_onode);
			if (disable) {
				newgene->enabled = false;
				disable = false;
			}
			vector_add(newgenes, newgene);
		}
	}

	//Return the baby Genome
	return (Genome_Init(genomeid, newtraits, newnodes, newgenes));
}

genome_t *Genome_Mate_Multipoint_Avg(genome_t *genome, genome_t *other, int genomeid, double fitness1, double fitness2, cbool interspec_flag)
{
	//The baby Genome will contain these new Traits, NNodes, and Genes
	vector *newtraits = vector_init(), *newnodes = vector_init(), *newgenes = vector_init();

	//iterators for moving through the two parents' genes
	neuron_t *inode;  //NNodes connected to the chosen Gene
	neuron_t *onode;
	int nodetraitnum;  //Trait number for a NNode

	//First, average the Traits from the 2 parents to form the baby's Traits
	//It is assumed that trait lists are the same length
	//In the future, may decide on a different method for trait mating
	for (int i = 0; i < genome->traits->count; i++)
		vector_add(newtraits, Trait_Init_Merge(genome->traits->data[i], other->traits->data[i]));

	//Figure out which genome is better
	//The worse genome should not be allowed to add extra structural baggage
	//If they are the same, use the smaller one's disjoint and excess genes only
	cbool p1better = (fitness1 > fitness2) ? true : (fitness1 == fitness2) ? (genome->genes->count < other->genes->count) : false;

	//NEW 3/17/03 Make sure all sensors and outputs are included
	for (int i = 0; i < other->neurons->count; i++)
	{
		neuron_t *curnode = other->neurons->data[i];
		if (curnode->node_label == NQ_INPUT ||
			curnode->node_label == NQ_OUTPUT ||
			curnode->node_label == NQ_BIAS)
		{
			nodetraitnum = (curnode->trait) ? (curnode->trait->id - ((trait_t*)genome->traits->data[0])->id) : 0;
			Genome_Node_Insert(genome, newnodes, Neuron_Init_Derived(curnode, newtraits->data[nodetraitnum]));
		}
	}

	gene_t *avgene = Gene_Init_Trait(0,0,0,0,0,0,0);

	//Now move through the Genes of each parent until both genomes end
	int loop_length = max(genome->genes->count, other->genes->count);
	for (int i = 0; i < loop_length; i++)
	{
		// Should we skip the excess genes in the larger of the two genomes?
		cbool skip = false;
		cbool disable = false; //Set to true if we want to disabled a chosen gene
		gene_t *chosengene = 0;  //Gene chosen for baby to inherit
		avgene->enabled = true;

		// Pick a gene to pass on to the child.
		if (i >= genome->genes->count) // We've run out of genes from out first genome. Use the second gene if it's better.
		{
			chosengene = other->genes->data[i];
			if (p1better) skip = true;  //Skip excess from the worse genome.
		}
		else if (i >= other->genes->count) // Vice versa
		{
			chosengene = genome->genes->data[i];
			if (!p1better) skip = true;  //Skip excess from the worse genome.
		}
		else
		{
			gene_t* p1gene = genome->genes->data[i];
			gene_t* p2gene = other->genes->data[i];

			double p1innov = p1gene->innovation_num;
			double p2innov = p2gene->innovation_num;

			if (p2innov == p1innov)
			{
				if (Random_Float() > 0.5) avgene->link->trait = p1gene->link->trait;
				else avgene->link->trait = p2gene->link->trait;

				//WEIGHTS AVERAGED HERE
				avgene->link->weight = (p1gene->link->weight + p2gene->link->weight) / 2.0;

				if (Random_Float() > 0.5) avgene->link->inode = p1gene->link->inode;
				else avgene->link->inode = p2gene->link->inode;

				if (Random_Float() > 0.5) avgene->link->onode = p1gene->link->onode;
				else avgene->link->onode = p2gene->link->onode;

				if (Random_Float() > 0.5) avgene->link->recurrent = p1gene->link->recurrent;
				else (avgene->link)->recurrent = p2gene->link->recurrent;

				avgene->innovation_num = p1gene->innovation_num;
				avgene->mutation_num = (p1gene->mutation_num + p2gene->mutation_num) / 2.0;

				//If one of the genes is disabled, the new one will likely also be.
				if ((!p1gene->enabled || !p2gene->enabled) && Random_Float(0.75)) avgene->enabled = false;

				chosengene = avgene;
			}
			else if (p1innov < p2innov)
			{
				chosengene = p1gene;
				if (!p1better) skip = true;
			}
			else if (p2innov < p1innov)
			{
				chosengene = p2gene;
				if (p1better) skip = true;
			}
		}
		/*
		//Uncomment this line to let growth go faster (from both parents excesses)
		skip=false;

		//For interspecies mating, allow all genes through:
		if (interspec_flag)
		skip=false;
		*/


		//Check to see if the chosengene conflicts with an already chosen gene
		//i.e. do they represent the same link    

		gene_t* checkedgene = 0;
		for (int j = 0; j < newgenes->count; j++) {
			//Check if they either share the same node IDs, or if their inputs and outputs are recursive and link between each other.
			if ((checkedgene->link->inode->node_id == chosengene->link->inode->node_id &&
				checkedgene->link->onode->node_id == chosengene->link->onode->node_id &&
				checkedgene->link->recurrent == chosengene->link->recurrent) ||
				(checkedgene->link->inode->node_id == chosengene->link->onode->node_id &&
				checkedgene->link->onode->node_id == chosengene->link->inode->node_id &&
				checkedgene->link->recurrent && chosengene->link->recurrent))
			{
				skip = true;
				break;
			}
		}


		//Now add the chosengene to the baby
		if (!skip)
		{
			neuron_t *new_inode = 0;
			neuron_t *new_onode = 0;

			int trait_num = 0;
			//Get the trait pointer.
			if (chosengene->link->trait == 0)
				trait_num = ((trait_t*)genome->traits->data[0])->id - 1;
			else //The subtracted number normalizes depending on whether traits start counting at 1 or 0
				trait_num = chosengene->link->trait->id - ((trait_t*)genome->traits->data[0])->id;

			//Next check for the nodes, add them if not in the baby Genome already

			//Swap the inode and onode depending on which will come first in the genome.s
			if (chosengene->link->inode->node_id < chosengene->link->onode->node_id)
			{
				inode = chosengene->link->inode;
				onode = chosengene->link->onode;
			}
			else
			{
				onode = chosengene->link->inode;
				inode = chosengene->link->onode;
			}
			//Checking for inode's existence
			neuron_t* curnode = 0;
			for (int j = 0; j < newnodes->count; j++)
			{
				if (((neuron_t*)newnodes->data[j])->node_id == inode->node_id)
				{
					curnode = newnodes->data[j];
					break;
				}
			}

			if (curnode == 0)
			{
				nodetraitnum = (inode->trait) ? inode->trait->id - (((trait_t*)genome->traits->data[0])->id) : 0;
				Genome_Node_Insert(genome, newnodes, Neuron_Init_Derived(inode, newtraits->data[nodetraitnum]));
			}
			else new_inode = curnode;

			//Getting onode this time.
			for (int j = 0; j < newnodes->count; j++)
			{
				if (((neuron_t*)newnodes->data[j])->node_id == onode->node_id)
				{
					curnode = newnodes->data[j];
					break;
				}
			}

			if (curnode == 0)
			{
				nodetraitnum = (onode->trait) ? onode->trait->id - (((trait_t*)genome->traits->data[0])->id) : 0;
				Genome_Node_Insert(genome, newnodes, Neuron_Init_Derived(onode, newtraits->data[nodetraitnum]));
			}
			else new_onode = curnode;


			//Add the Gene
			gene_t* newgene = Gene_Init_Dupe(chosengene, newtraits->data[trait_num], new_inode, new_onode);
			if (disable) {
				newgene->enabled = false;
				disable = false;
			}
			vector_add(newgenes, newgene);
		}
	}

	Gene_Delete(avgene);

	//Return the baby Genome
	return (Genome_Init(genomeid, newtraits, newnodes, newgenes));
}

genome_t *Genome_Mate_Singlepoint(genome_t *genome, genome_t *other, int genomeid)
{
	//The baby Genome will contain these new Traits, NNodes, and Genes
	vector *newtraits = vector_init(), *newnodes = vector_init(), *newgenes = vector_init();

	//iterators for moving through the two parents' genes
	neuron_t *inode;  //NNodes connected to the chosen Gene
	neuron_t *onode;
	int nodetraitnum;  //Trait number for a NNode

	//First, average the Traits from the 2 parents to form the baby's Traits
	//It is assumed that trait lists are the same length
	//In the future, may decide on a different method for trait mating
	for (int i = 0; i < genome->traits->count; i++)
		vector_add(newtraits, Trait_Init_Merge(genome->traits->data[i], other->traits->data[i]));

	gene_t *avgene = Gene_Init_Trait(0, 0, 0, 0, 0, 0, 0);

	//NEW 3/17/03 Make sure all sensors and outputs are included
	for (int i = 0; i < other->neurons->count; i++)
	{
		neuron_t *curnode = other->neurons->data[i];
		if (curnode->node_label == NQ_INPUT ||
			curnode->node_label == NQ_OUTPUT ||
			curnode->node_label == NQ_BIAS)
		{
			nodetraitnum = (curnode->trait) ? (curnode->trait->id - ((trait_t*)genome->traits->data[0])->id) : 0;
			Genome_Node_Insert(genome, newnodes, Neuron_Init_Derived(curnode, newtraits->data[nodetraitnum]));
		}
	}

	int crosspoint = Random_Int(0, min(genome->genes->count, other->genes->count) - 1);
	int geneCount = 0;

	//Now move through the Genes of each parent until both genomes end
	int loop_length = max(genome->genes->count, other->genes->count);
	for (int i = 0; i < loop_length; i++)
	{
		// Should we skip the excess genes in the larger of the two genomes?
		cbool skip = false;
		cbool disable = false; //Set to true if we want to disabled a chosen gene
		gene_t *chosengene = 0;  //Gene chosen for baby to inherit
		avgene->enabled = true;

		// Pick a gene to pass on to the child.
		if (i >= genome->genes->count) // We've run out of genes from out first genome. Use the second gene if it's better.
		{
			chosengene = other->genes->data[i];
		}
		else if (i >= other->genes->count) // Vice versa
		{
			chosengene = genome->genes->data[i];
		}
		else
		{
			gene_t* p1gene = genome->genes->data[i];
			gene_t* p2gene = other->genes->data[i];

			double p1innov = p1gene->innovation_num;
			double p2innov = p2gene->innovation_num;

			if (p2innov == p1innov)
			{

				//Pick the chosengene depending on whether we've crossed yet.
				if (i < crosspoint)
					chosengene = p1gene;
				else if (i > crosspoint)
					chosengene = p2gene;
				else
				{
					if (Random_Float() > 0.5) avgene->link->trait = p1gene->link->trait;
					else avgene->link->trait = p2gene->link->trait;

					//WEIGHTS AVERAGED HERE
					avgene->link->weight = (p1gene->link->weight + p2gene->link->weight) / 2.0;

					if (Random_Float() > 0.5) avgene->link->inode = p1gene->link->inode;
					else avgene->link->inode = p2gene->link->inode;

					if (Random_Float() > 0.5) avgene->link->onode = p1gene->link->onode;
					else avgene->link->onode = p2gene->link->onode;

					if (Random_Float() > 0.5) avgene->link->recurrent = p1gene->link->recurrent;
					else (avgene->link)->recurrent = p2gene->link->recurrent;

					avgene->innovation_num = p1gene->innovation_num;
					avgene->mutation_num = (p1gene->mutation_num + p2gene->mutation_num) / 2.0;

					//If one of the genes is disabled, the new one will likely also be.
					if ((!p1gene->enabled || !p2gene->enabled) && Random_Float(0.75)) avgene->enabled = false;

					chosengene = avgene;
				}
			}
			else if (p1innov < p2innov)
			{
				if (i < crosspoint)
				{
					chosengene = p1gene;
				}
				else
				{
					chosengene = p2gene;
				}
			}
			else if (p2innov < p1innov)
			{
				skip = true;
			}
		}
		/*
		//Uncomment this line to let growth go faster (from both parents excesses)
		skip=false;

		//For interspecies mating, allow all genes through:
		if (interspec_flag)
		skip=false;
		*/


		//Check to see if the chosengene conflicts with an already chosen gene
		//i.e. do they represent the same link    

		gene_t* checkedgene = 0;
		for (int j = 0; j < newgenes->count; j++) {
			//Check if they either share the same node IDs, or if their inputs and outputs are recursive and link between each other.
			if ((checkedgene->link->inode->node_id == chosengene->link->inode->node_id &&
				checkedgene->link->onode->node_id == chosengene->link->onode->node_id &&
				checkedgene->link->recurrent == chosengene->link->recurrent) ||
				(checkedgene->link->inode->node_id == chosengene->link->onode->node_id &&
				checkedgene->link->onode->node_id == chosengene->link->inode->node_id &&
				checkedgene->link->recurrent && chosengene->link->recurrent))
			{
				skip = true;
				break;
			}
		}


		//Now add the chosengene to the baby
		if (!skip)
		{
			neuron_t *new_inode = 0;
			neuron_t *new_onode = 0;

			int trait_num = 0;
			//Get the trait pointer.
			if (chosengene->link->trait == 0)
				trait_num = ((trait_t*)genome->traits->data[0])->id - 1;
			else //The subtracted number normalizes depending on whether traits start counting at 1 or 0
				trait_num = chosengene->link->trait->id - ((trait_t*)genome->traits->data[0])->id;

			//Next check for the nodes, add them if not in the baby Genome already

			//Swap the inode and onode depending on which will come first in the genome.
			if (chosengene->link->inode->node_id < chosengene->link->onode->node_id)
			{
				inode = chosengene->link->inode;
				onode = chosengene->link->onode;
			}
			else
			{
				onode = chosengene->link->inode;
				inode = chosengene->link->onode;
			}
			//Checking for inode's existence
			neuron_t* curnode = 0;
			for (int j = 0; j < newnodes->count; j++)
			{
				if (((neuron_t*)newnodes->data[j])->node_id == inode->node_id)
				{
					curnode = newnodes->data[j];
					break;
				}
			}

			if (curnode == 0)
			{
				nodetraitnum = (inode->trait) ? inode->trait->id - (((trait_t*)genome->traits->data[0])->id) : 0;
				Genome_Node_Insert(genome, newnodes, Neuron_Init_Derived(inode, newtraits->data[nodetraitnum]));
			}
			else new_inode = curnode;

			//Getting onode this time.
			for (int j = 0; j < newnodes->count; j++)
			{
				if (((neuron_t*)newnodes->data[j])->node_id == onode->node_id)
				{
					curnode = newnodes->data[j];
					break;
				}
			}

			if (curnode == 0)
			{
				nodetraitnum = (onode->trait) ? onode->trait->id - (((trait_t*)genome->traits->data[0])->id) : 0;
				Genome_Node_Insert(genome, newnodes, Neuron_Init_Derived(onode, newtraits->data[nodetraitnum]));
			}
			else new_onode = curnode;


			//Add the Gene
			vector_add(newgenes, Gene_Init_Dupe(chosengene, newtraits->data[trait_num], new_inode, new_onode));
		}
	}

	Gene_Delete(avgene);

	//Return the baby Genome
	return (Genome_Init(genomeid, newtraits, newnodes, newgenes));
}

double Genome_Compatibility(genome_t *genome, genome_t *other)
{
	//Set up the counters
	double num_disjoint = 0.0;
	double num_excess = 0.0;
	double mut_diff_total = 0.0;
	double num_matching = 0.0;  //Used to normalize mutation_num differences
	
	int loop_length = max(genome->genes->count, other->genes->count);
	for (int i = 0; i < loop_length; i++)
	{
		if (i >= genome->genes->count || i >= other->genes->count) {
			num_excess += 1.0;
		}
		else {
			gene_t *p1gene = genome->genes->data[i];
			gene_t *p2gene = other->genes->data[i];

			//Extract current innovation numbers
			double p1innov = p1gene->innovation_num;
			double p2innov = p2gene->innovation_num;

			if (p1innov == p2innov) 
			{
				num_matching += 1.0;
				//mut_diff+=trait_compare((*p1gene)->lnk->linktrait,(*p2gene)->lnk->linktrait); //CONSIDER TRAIT DIFFERENCES
				mut_diff_total += fabs(p1gene->mutation_num - p2gene->mutation_num);
			}
			else if (p1innov < p2innov || p2innov < p1innov)
			{
				num_disjoint += 1.0;
			}
		}
	}

	return (NQ_DISJOINT_COEFF * (num_disjoint / 1.0) +
		NQ_EXCESS_COEFF * (num_excess / 1.0) +
		NQ_MUTDIFF_COEFF * (mut_diff_total / num_matching));
}

double Genome_Trait_Compare(genome_t *genome, trait_t *t1, trait_t *t2)
{
	if ((t1->id == 1 && t2->id >= 2) || (t2->id == 1 && t1->id >= 2))
		return 0.5;
	else if (t1->id >= 2) 
	{
		double params_diff = 0.0;
		for (int i = 0; i <= 2; i++) {
			params_diff += fabs(t1->params[i] - t2->params[i]);
		}
		return params_diff / 4.0;
	}
	return 0.0;
}

int Genome_Extrons(genome_t *genome)
{
	int total = 0;
	for (int i = 0; i < genome->genes->count; i++)
		if (((gene_t*)genome->genes->data[i])->enabled)
			total++;
	return total;
}

void Genome_Randomize_Traits(genome_t *genome)
{
	for (int i = 0; i < genome->neurons->count; i++)
	{
		neuron_t *curnode = genome->neurons->data[i];
		int trait_num = Random_Int(1, genome->traits->count);
		curnode->trait_id = trait_num;

		trait_t *curtrait = 0;
		for (int j = 0; j < genome->traits->count; j++)
		{
			curtrait = genome->traits->data[j];
			if (curtrait->id == trait_num) break;
		}
		curnode->trait = curtrait;
	}

	for (int i = 0; i < genome->genes->count; i++)
	{
		gene_t *curgene = genome->genes->data[i];
		int trait_num = Random_Int(1, genome->traits->count);
		curgene->link->trait_id = trait_num;

		trait_t *curtrait = 0;
		for (int j = 0; j < genome->traits->count; j++)
		{
			curtrait = genome->traits->data[j];
			if (curtrait->id == trait_num) break;
		}
		curgene->link->trait = curtrait;
	}
}


void Genome_Node_Insert(genome_t *genome, vector *nlist, neuron_t *n)
{
	for (int i = 0; i < nlist->count; i++)
	{
		neuron_t* curnode = nlist->data[i];
		if (curnode->node_id >= n->node_id) 
			vector_insert(nlist, i, n);
	}
}

void Genome_Add_Gene(genome_t *genome, vector *glist, gene_t *g)
{
	for (int i = 0; i < glist->count; i++)
	{
		gene_t* curgene = glist->data[i];
		if (curgene->innovation_num >= g->innovation_num)
			vector_insert(glist, i, g);
	}
}