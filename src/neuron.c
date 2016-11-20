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
#include "neuron.h"
#include "gene.h"
#include "neural_def.h"
#include "quakedef.h"
#include <stdlib.h>

neuron_t* Neuron_Init()
{
	neuron_t* neuron = malloc(sizeof(neuron_t));
	if (neuron == 0) return ((void*)1);

	neuron->value = 0;
	neuron->ilinks = vector_init();
	neuron->olinks = vector_init();

	return neuron;
}

neuron_t* Neuron_Init_Derived(neuron_t* other)
{
	neuron_t* neuron = malloc(sizeof(neuron_t));
	if (neuron == 0) return ((void*)1);

	neuron->value = other->value;
	neuron->ilinks = vector_init();
	neuron->olinks = vector_init();

	return neuron;
}

/*
neuron_t* Neuron_Init_Load(char *argline)
{
	char *curword;
	int args[3] = { -1, -1, -1 }; // Args: ID, Type, Placement.

	char wordbuf[1024];
	strcpy(wordbuf, argline);

	// Read the line into memory.
	curword = strtok(wordbuf, " \n");

	// n denotes node information, and should always be present as the first word.
	if (strcmp(curword, "n") != 0)
	{
		Con_Printf("Erroneus argline passed to node [%s]!", argline);
		return 0;
	}

	for (int i = 0; i < 3; i++)
	{
		// Read the next word.
		curword = strtok(NULL, " \n");

		// Error handling.
		if (curword == NULL)
		{
			Con_Printf("Error loading neuron #%s value %d!", args[1], i);
			return 0;
		}
		// Convert each argument to its decimal equivalent.
		sscanf(curword, "%d", &args[i]);
	}

	// We can just pass these value into the other constructor now.
	return Neuron_Init();
}

double Neuron_Get_Active_Out(neuron_t* node)
{
	return (node->activation_count > 0) ? node->activation : 0.0;
}

double Neuron_Get_Active_Out_TD(neuron_t* node)
{
	return (node->activation_count > 0) ? node->last_activation : 0.0;
}
*/

void Neuron_Delete(neuron_t* node)
{
	for (int i = 0; i < node->ilinks->count; i++)
		Gene_Delete(node->ilinks->data[i]);

	free(node);
}

/*
cbool Neuron_Sensor_Load(neuron_t* node, double value)
{
	node->value = value;
}

void Neuron_Add_Incoming_Recurring(neuron_t* node, neuron_t* other, double w, cbool recur)
{
	gene_t* newlink = Link_Init(w, other, node, recur);
	vector_add(node->ilinks, newlink);
	vector_add(other->olinks, newlink);
}

void Neuron_Add_Incoming(neuron_t* node, neuron_t* other, double w)
{
	gene_t* newlink = Link_Init(w, other, node, false);
	vector_add(node->ilinks, newlink);
	vector_add(other->olinks, newlink);
}

void Neuron_Flushback(neuron_t* node)
{
	if (node->type != NQ_SENSOR)
	{
		if (node->activation_count > 0)
		{
			node->activation_count = 0;
			node->activation = 0;
			node->last_activation = 0;
			node->last_activation2 = 0;
		}

		for (int i = 0; i < node->ilinks->count; i++)
		{
			gene_t* curlink = node->ilinks->data[i];
			//curlink->added_weight = 0;
			if (curlink->inode->activation_count > 0)
				Neuron_Flushback(curlink->inode);
		}
	}
	else
	{
		node->activation_count = 0;
		node->activation = 0;
		node->last_activation = 0;
		node->last_activation2 = 0;
	}
}

void Neuron_Flushback_Check(neuron_t* node, vector* seenlist)
{
	if (node->activation_count > 0)
	{
		printf("ALERT: Node %d has activation count %d\n", node->id, node->activation_count);
	}
	if (node->activation > 0)
	{
		printf("ALERT: Node %d has activation %d\n", node->id, node->activation);
	}
	if (node->last_activation > 0)
	{
		printf("ALERT: Node %d has last_activation %d\n", node->id, node->last_activation);
	}
	if (node->last_activation2 > 0)
	{
		printf("ALERT: Node %d has last_activation2 %d\n", node->id, node->last_activation2);
	}

	if (!(node->type == NQ_SENSOR)) {

		for (int i = 0; i < node->ilinks->count; i++)
		{
			neuron_t* location = 0;
			gene_t* curlink = node->ilinks->data[i];
			for (int j = 0; j < seenlist->count; j++)
			{
				if (seenlist->data[j] == curlink->inode)
				{
					location = seenlist->data[j];
					break;
				}
			}
			if (location == 0)
			{
				vector_add(seenlist, curlink->inode);
				Neuron_Flushback_Check(curlink->inode, seenlist);
			}
			curlink->added_weight = 0;
			if (curlink->inode->activation_count > 0)
				Neuron_Flushback(curlink->inode);
		}

	}
}

void Neuron_Derive_Trait(neuron_t* node, trait_t *curtrait)
{
	if (curtrait != 0) {
		for (int i = 0; i < NQ_TRAIT_NUM_PARAMS; i++)
			node->params[i] = (curtrait->params)[i];
	}
	else {
		for (int i = 0; i < NQ_TRAIT_NUM_PARAMS; i++)
			node->params[i] = 0;
	}

	if (curtrait != 0)
		node->trait_id = curtrait->id;
	else node->trait_id = 1;
}

void Neuron_Override_Output(neuron_t* node, double new_output)
{
	node->override_value = new_output;
	node->override = true;
}

void Neuron_Activate_Override(neuron_t* node)
{
	node->activation = node->override_value;
	node->override = false;
}


int Neuron_Depth(neuron_t* node, int d)
{
	if (d > 100) return 10;
	//if (node->type == NQ_SENSOR) return d;

	int max = d;

	for (int i = 0; i < node->ilinks->count; i++)
	{
		gene_t* curlink = node->ilinks->data[i];
		int cur_depth = Neuron_Depth(curlink->inode, d + 1);
		if (cur_depth > max) max = cur_depth;
	}

	return max;
}

/*
void Neuron_FPrint(neuron_t *node, FILE *f)
{
	fprintf(f, "n %d %d %d\n", node, node->type, node->node_label);
}
*/