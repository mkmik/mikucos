#ifndef __IO_H__
#define __IO_H__

// stolen from LINUX

// output

static __inline void
outb (unsigned char value, unsigned short int port) {
  __asm__ __volatile__ ("outb %b0,%w1": :"a" (value), "Nd" (port));
}

static __inline void
outb_p (unsigned char value, unsigned short int port) {
  __asm__ __volatile__ ("outb %b0,%w1\noutb %%al,$0x80": :"a" (value),
                        "Nd" (port));
}

static __inline void
outw (unsigned short int value, unsigned short int port)
{
  __asm__ __volatile__ ("outw %w0,%w1": :"a" (value), "Nd" (port));
}

static __inline void
outw_p (unsigned short int value, unsigned short int port) {
  __asm__ __volatile__ ("outw %w0,%w1\noutb %%al,$0x80": :"a" (value),
                        "Nd" (port));
}

static __inline void
outl (unsigned long int value, unsigned short int port)
{
  __asm__ __volatile__ ("outl %0,%1": :"a" (value), "Nd" (port));
}

// input 

static __inline unsigned char
inb (unsigned short int port)
{
  unsigned char _v;

  __asm__ __volatile__ ("inb %w1,%0":"=a" (_v):"Nd" (port));
  return _v;
}

static __inline unsigned short int
inw (unsigned short int port)
{
  unsigned short int _v;

  __asm__ __volatile__ ("inw %w1,%0":"=a" (_v):"Nd" (port));
  return _v;
}

static __inline unsigned long int
inl (unsigned short int port)
{
  unsigned long int _v;

  __asm__ __volatile__ ("inl %1,%0":"=a" (_v):"Nd" (port));
  return _v;
}

#endif
