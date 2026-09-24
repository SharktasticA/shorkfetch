/* Copyright (c) 2012 Artem Shinkarov <artyom.shinkaroff@gmail.com>
   Modified by Redjard (c) 2026 under GPLv3.
   Original copyright attached below, original source available at:
   https://github.com/ashinkarov/trie

   Permission to use, copy, modify, and distribute this software for any
   purpose with or without fee is hereby granted, provided that the above
   copyright notice and this permission notice appear in all copies.
   THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
   WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
   MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
   ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
   WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
   ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
   OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.  */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>

#include "trie.h"

/* Allocate a new empty trie.  */
struct trie * trie_new() {
	struct trie * trie = (struct trie *) malloc(sizeof(struct trie));
	trie->children_size = TRIE_CHILDREN;
	trie->children_count = 0;
	trie->children = (struct child *) malloc(TRIE_CHILDREN * sizeof (struct child));
	memset( trie->children, 0, TRIE_CHILDREN*sizeof(struct child) );
	return trie;
}
// trie.children[] -> trie.children[] -> ...


/* Helper for bsearch and qsort.  */
static inline int cmp_children( const void* k1, const void* k2 ) {
	struct child *  c1 = (struct child *)k1;
	struct child *  c2 = (struct child *)k2;
	return c1->symb - c2->symb;
}

static inline int child_is_endnode(const struct child* child) {
	return child->next == NULL;
}


/* internal: fetch matching direct child of trie */
static struct child* trie_search_child(const struct trie * trie, int symb)
{
	if (trie->children_count == 0)
		return NULL;

	struct child s;
	s.symb = symb;
	
	return (struct child *) bsearch(&s, trie->children, trie->children_count, sizeof(struct child), cmp_children);
}

/* Add a prefix to the trie.  */
int _trie_add_word(struct trie * trie, const char * word, size_t length, ssize_t info)
{
	
	struct child* child;
	child = trie_search_child(trie, word[0]);  // Null or child that has same symbol as first char
	
	struct trie* next;
	if (child) {
		next = child->next;
		
		if (length == 1 || child_is_endnode(child))
			return 0;
	} else {
		// init new child
		
		// make sure we have enough space allocated in the array
		if (trie->children_count >= trie->children_size) {
			trie->children_size *= 2;
			trie->children = (struct child *) realloc( trie->children, trie->children_size*sizeof(struct child) );
		}
		
		child = &trie->children[trie->children_count++];
		child->symb = word[0];
		if (length > 1) {
			// init as intermediate node
			child->next = next = trie_new();
		} else {
			// init as end node
			child->next = NULL;
			child->last = info;
		}
		
		// /* XXX This qsort may not perform ideally, as actually we are always
		//  just shifting a number of elements a the end of the array one
		//  element to the left.  Possibly qsort, can figure it out and work
		//  in O (N) time.  Otherwise better alternative is needed.  */
		qsort( trie->children, trie->children_count, sizeof (struct child), cmp_children );
		
		// if we added an endnode, that means we are done
		if (length == 1)
			return 1;
	}
	
	return _trie_add_word(next, &word[1], length - 1, info);
}
void trie_add_word(struct trie * trie, const char * word, size_t length, ssize_t info) {
	assert (trie != NULL);
	if (!_trie_add_word(trie, word, length, info)) {
		printf("Warning: tried adding duplicate or conflicting entry to radix trie: ");
		fwrite(word,sizeof(*word),length,stdout);
		printf("\n");
		return;
	};
}


/* Print the trie.  */
static void _trie_print (struct trie *  t, int level) {
	if (!t)
		return;

	for (unsigned int i = 0; i < t->children_count; i++){
		for (int ii = 0; ii < level; ii++)
			printf ("  ");
		printf("%c", (char) t->children[i].symb);
		if (child_is_endnode(&t->children[i]))
			printf(" [%li]", t->children[i].last);
		printf("\n");
		_trie_print(t->children[i].next, level+1);
	}
}
/* Wrapper for print.  */
void trie_print (struct trie *  t) {
	_trie_print (t, 0);
}


/* Deallocate memory used for trie.  */
void
trie_free (struct trie *  trie)
{
	unsigned int  i;
	if (!trie)
		return;

	for (i = 0; i < trie->children_count; i++)
		trie_free (trie->children[i].next);

	if (trie->children)
		free (trie->children);
	free (trie);
}

/* Search for word in trie.  Returns true/false.  */
ssize_t trie_search(const struct trie *  trie, const char *  word, size_t length) {
	struct child* child;

	assert (length > 0);
	if (trie == NULL)
		return ELEMENT_NOT_FOUND;

	child = trie_search_child(trie, word[0]);

	if (!child)
		return ELEMENT_NOT_FOUND;
	
	if (child_is_endnode(child))
		return child->last;
	
	return trie_search (child->next, &word[1], length - 1);
}
