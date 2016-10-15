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
#ifndef __GENE_H__
#define __GENE_H__

#include <stdio.h>
#include "neural_def.h"
#include "environment.h"
#include "trait.h"
#include "link.h"
#include "network.h"

//Type definitions for our neural network.
typedef struct gene_s
{
	double innovation_num; // Used for finding corresponding genes during crossover.
	double mutation_num; // Used to see how much mutation has changed the link.

	cbool enabled; // Disables the gene.
	cbool frozen; // Stops the gene from mutating.

	link_t *link; // Genetic link data.
} gene_t; // Defines the connection between neurons.

// Construct a gene without a trait.
gene_t Gene_Init(double w, neuron_t* in_node, neuron_t* out_node, cbool recurring, double innov, double mnum);

// Construct a gene with a trait.
gene_t Gene_Init_Trait(trait_t* trait, double w, neuron_t* in_node, neuron_t* out_node, cbool recurring, double innov, double mnum);

// Duplicate a gene from another existing gene.
gene_t Gene_Init_Copy(const gene_t g);

// Construct a gene from a file spec given traits and nodes.
gene_t Gene_Init_File(const char *argline, vector traits, vector nodes);

// Delete a gene
void Gene_Delete();

// Print gene to a file. Called from Genome.
void Gene_Print(FILE *outFile);

#endif // !__GENE_H__