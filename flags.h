#include <stdint.h>

#ifndef FLAGS_H
#define FLAGS_H

void set_Flag(uint32_t f);
void clear_Flag(uint32_t f);
int test_Flag(uint32_t f);
int parity(uint8_t v);
int zero(uint8_t v);
int sign(uint8_t v);

/*EFLAGS     0b0xxxxxxxxx0x0x1x    16-bits   */
#define CF   0b0000000000000001 /* bit 0     */
/* Always set to 1                 bit 1     */
#define PF   0b0000000000000100 /* bit 2     */
/* Always set to 0                 bit 3     */
#define AF   0b0000000000010000 /* bit 4     */
/* Always set to 0                 bit 5     */
#define ZF   0b0000000001000000 /* bit 6     */
#define SF   0b0000000010000000 /* bit 7     */
#define TF   0b0000000100000000 /* bit 8     */
#define IF   0b0000001000000000 /* bit 9     */
#define DF   0b0000010000000000 /* bit 10    */
#define OF   0b0000100000000000 /* bit 11    */
#define IOPL 0b0011000000000000 /* bit 12-13 */
#define NT   0b0100000000000000 /* bit 14    */
/* Always set to 0                 bit 15    */
#define RF 0b010000000000000000 /* bit 16    */
#define VM 0b100000000000000000 /* bit 17    */

#endif
