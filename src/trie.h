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

#ifndef __TRIE_H__
#define __TRIE_H__

/* Deafult number of the number of children in trie node.
   It is a good idea to pick the value being power of two
   as when the number of children has to be increase the
   number will be ultiplied by 2, and further operarions
   could be optimized to shifts rather than multiplications.  */
#define TRIE_CHILDREN 1


/* Returned when trie_search finds no match  */
#define ELEMENT_NOT_FOUND -1


/* Structure to search keywords.  The search complexity is proportional to
   LW * log P, where LW is the length of the word, and P is an average number
   of words sharing the same prefix.  As an upper aproximation P is a length
   of the alphabet.

   The benefit of the structure is that with the same complexity one can
   get all the possible endings of a certain prefix.  */
struct child;
struct trie {
  unsigned int children_size;
  unsigned int children_count;
  struct child* children;
};

struct child {
  int symb;
  ssize_t last;
  struct trie* next;
};

__BEGIN_DECLS
struct trie* trie_new (void);
void trie_add_word (struct trie *, const char *, size_t, ssize_t);
void trie_print (struct trie *);
void trie_free (struct trie *);
ssize_t trie_search (const struct trie *, const char *, size_t);
__END_DECLS

#endif  /* __TRIE_H__  */
