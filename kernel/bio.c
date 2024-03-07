// Buffer cache.
//
// The buffer cache is a linked list of buf structures holding
// cached copies of disk block contents.  Caching disk blocks
// in memory reduces the number of disk reads and also provides
// a synchronization point for disk blocks used by multiple processes.
//
// Interface:
// * To get a buffer for a particular disk block, call bread.
// * After changing buffer data, call bwrite to write it to disk.
// * When done with the buffer, call brelse.
// * Do not use the buffer after calling brelse.
// * Only one process at a time can use a buffer,
//     so do not keep them longer than necessary.


#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "riscv.h"
#include "defs.h"
#include "fs.h"
#include "buf.h"

//my
#define NBUCKET 13
//
// struct {
//   struct spinlock lock;
//   struct buf buf[NBUF];

//   // Linked list of all buffers, through prev/next.
//   // Sorted by how recently the buffer was used.
//   // head.next is most recent, head.prev is least.
//   struct buf head;
// } bcache;
struct {
  struct spinlock lock[NBUCKET]; //锁
  struct buf buetets[NBUCKET];  //哈希桶
  struct buf buf[NBUF]; //所有缓存块，每个桶可以存储NBUF
  struct spinlock eviction_lock;
} bcache;
// void
// binit(void)
// {
//   struct buf *b;

//   initlock(&bcache.lock, "bcache");

//   // Create linked list of buffers
//   bcache.head.prev = &bcache.head;
//   bcache.head.next = &bcache.head;
//   for(b = bcache.buf; b < bcache.buf+NBUF; b++){
//     b->next = bcache.head.next;
//     b->prev = &bcache.head;
//     initsleeplock(&b->lock, "buffer");
//     bcache.head.next->prev = b;
//     bcache.head.next = b;
//   }
// }
void
binit(void)
{
  struct buf *b;
  initlock(&bcache.eviction_lock, "bcache_eviction_lock");
  for(int i=0;i<NBUCKET;++i){
    initlock(&bcache.lock[i], "bcache");
    // Create linked list of buffers
    bcache.buetets[i].prev = &bcache.buetets[i];
    bcache.buetets[i].next = &bcache.buetets[i];
  }
  for(b = bcache.buf; b < bcache.buf+NBUF; b++){
    b->next = bcache.buetets[0].next;
    b->prev = &bcache.buetets[0];
    initsleeplock(&b->lock, "buffer");
    bcache.buetets[0].next->prev = b;
    bcache.buetets[0].next = b;
  }
}
// Look through buffer cache for block on device dev.
// If not found, allocate a buffer.
// In either case, return locked buffer.

static struct buf*
bget(uint dev, uint blockno)
{
  struct buf *b;
  int bucketIndex=(dev+blockno)%NBUCKET;
  acquire(&bcache.lock[bucketIndex]);

  // Is the block already cached?
  for(b = bcache.buetets[bucketIndex].next; b != &bcache.buetets[bucketIndex]; b = b->next){
    if(b->dev == dev && b->blockno == blockno){
      b->refcnt++;
      release(&bcache.lock[bucketIndex]);
      acquiresleep(&b->lock);
      return b;
    }
  }
  // Not cached.
  release(&bcache.lock[bucketIndex]);
  acquire(&bcache.eviction_lock);
  for(b = bcache.buetets[bucketIndex].next; b != &bcache.buetets[bucketIndex]; b = b->next){
    if(b->dev == dev && b->blockno == blockno){
      acquire(&bcache.lock[bucketIndex]);
      b->refcnt++;
      release(&bcache.lock[bucketIndex]);
      release(&bcache.eviction_lock);
      acquiresleep(&b->lock);
      return b;
    }
  }

  for(int i=(bucketIndex+1)%NBUCKET;i!=bucketIndex;i=(i+1)%NBUCKET){
    acquire(&bcache.lock[i]);
    for(b = bcache.buetets[i].prev; b != &bcache.buetets[i]; b = b->prev){
      if(b->refcnt == 0) {
        b->dev = dev;
        b->blockno = blockno;
        b->valid = 0;
        b->refcnt = 1;
        //窃取
        b->next->prev=b->prev;
        b->prev->next=b->next;
        release(&bcache.lock[i]);
        acquire(&bcache.lock[bucketIndex]);
        b->prev=&bcache.buetets[bucketIndex];
        b->next=bcache.buetets[bucketIndex].next;
        bcache.buetets[bucketIndex].next->prev=b;
        bcache.buetets[bucketIndex].next=b;
        release(&bcache.lock[bucketIndex]);
        release(&bcache.eviction_lock);
        acquiresleep(&b->lock);
        return b;
      }
    }
    release(&bcache.lock[i]);
  }

  panic("bget: no buffers");
}

// static struct buf*
// bget(uint dev, uint blockno)
// {
//   struct buf *b;
//   int bucketIndex=(dev+blockno)%NBUCKET;
//   acquire(&bcache.lock[bucketIndex]);

//   // Is the block already cached?
//   for(b = bcache.buetets[bucketIndex].next; b != &bcache.buetets[bucketIndex]; b = b->next){
//     if(b->dev == dev && b->blockno == blockno){
//       b->refcnt++;
//       release(&bcache.lock[bucketIndex]);
//       acquiresleep(&b->lock);
//       return b;
//     }
//   }
//   // Not cached.
//   // release(&bcache.lock[bucketIndex]);
//   acquire(&bcache.eviction_lock);
//   // Recycle the least recently used (LRU) unused buffer.
//   for(int i=(bucketIndex+1)%NBUCKET;i!=bucketIndex;i=(i+1)%NBUCKET){
//     acquire(&bcache.lock[i]);
//     for(b = bcache.buetets[i].prev; b != &bcache.buetets[i]; b = b->prev){
//       if(b->refcnt == 0) {
//         b->dev = dev;
//         b->blockno = blockno;
//         b->valid = 0;
//         b->refcnt = 1;
//         //窃取
//         b->next->prev=b->prev;
//         b->prev->next=b->next;
//         release(&bcache.lock[i]);
//         // acquire(&bcache.lock[bucketIndex]);
//         b->prev=&bcache.buetets[bucketIndex];
//         b->next=bcache.buetets[bucketIndex].next;
//         bcache.buetets[bucketIndex].next->prev=b;
//         bcache.buetets[bucketIndex].next=b;
//         release(&bcache.lock[bucketIndex]);
//         release(&bcache.eviction_lock);
//         acquiresleep(&b->lock);
//         return b;
//       }
//     }
//     release(&bcache.lock[i]);
//   }

//   panic("bget: no buffers");
// }






// Return a locked buf with the contents of the indicated block.
struct buf*
bread(uint dev, uint blockno)
{
  struct buf *b;

  b = bget(dev, blockno);
  if(!b->valid) {
    virtio_disk_rw(b, 0);
    b->valid = 1;
  }
  return b;
}

// Write b's contents to disk.  Must be locked.
void
bwrite(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("bwrite");
  virtio_disk_rw(b, 1);
}

// Release a locked buffer.
// Move to the head of the most-recently-used list.
// void
// brelse(struct buf *b)
// {
//   if(!holdingsleep(&b->lock))
//     panic("brelse");

//   releasesleep(&b->lock);
//   acquire(&bcache.lock);
//   b->refcnt--;
//   if (b->refcnt == 0) {
//     // no one is waiting for it.
//     b->next->prev = b->prev;
//     b->prev->next = b->next;
//     b->next = bcache.head.next;
//     b->prev = &bcache.head;
//     bcache.head.next->prev = b;
//     bcache.head.next = b;
//   }
  
//   release(&bcache.lock);
// }
void
brelse(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("brelse");

  releasesleep(&b->lock);
  int bucketIndex=(b->dev+b->blockno)%NBUCKET;
  acquire(&bcache.lock[bucketIndex]);
  b->refcnt--;
  if (b->refcnt == 0) {
    // no one is waiting for it.
    b->next->prev = b->prev;
    b->prev->next = b->next;
    b->next = bcache.buetets[bucketIndex].next;
    b->prev = &bcache.buetets[bucketIndex];
    bcache.buetets[bucketIndex].next->prev = b;
    bcache.buetets[bucketIndex].next = b;
  }
  
  release(&bcache.lock[bucketIndex]);
}
// void
// bpin(struct buf *b) {
//   acquire(&bcache.lock);
//   b->refcnt++;
//   release(&bcache.lock);
// }

// void
// bunpin(struct buf *b) {
//   acquire(&bcache.lock);
//   b->refcnt--;
//   release(&bcache.lock);
// }


void
bpin(struct buf *b) {
  int bucketIndex=(b->dev+b->blockno)%NBUCKET;
  acquire(&bcache.lock[bucketIndex]);
  b->refcnt++;
  release(&bcache.lock[bucketIndex]);
}

void
bunpin(struct buf *b) {
  int bucketIndex=(b->dev+b->blockno)%NBUCKET;
  acquire(&bcache.lock[bucketIndex]);
  b->refcnt--;
  release(&bcache.lock[bucketIndex]);
}