/*****************************************************
 * Copyright Grégory Mounié 2008-2018                *
 * This code is distributed under the GLPv3+ licence.*
 * Ce code est distribué sous la licence GPLv3+.     *
 *****************************************************/

#ifndef MEM_H
#define MEM_H

/* ENTETE DU TP1. NE RIEN INSERER OU EDITER DANS CE FICHIER ! */
/* TP1 HEADERS. DO NOT EDIT THIS FILE ! */


#ifdef __cplusplus
extern "C" {
#endif

void *emalloc(unsigned long user_request_size);
void efree(void *ptr);

#ifdef __cplusplus
}
#endif

#endif