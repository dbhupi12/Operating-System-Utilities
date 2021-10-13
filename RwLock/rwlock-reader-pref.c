#include "rwlock.h"

void InitalizeReadWriteLock(struct read_write_lock * rw)
{
  rw->readers = 0;
  sem_init(&rw->lock,0,1);
  sem_init(&rw->writelock,0,1);
  rw->writers = 0;
}

void ReaderLock(struct read_write_lock * rw)
{
  sem_wait(&rw->lock);
  rw->readers++;
  if(rw->readers == 1){
    sem_wait(&rw->writelock);
  }
  sem_post(&rw->lock);
}

void ReaderUnlock(struct read_write_lock * rw)
{
  sem_wait(&rw->lock);
  rw->readers--;
  if(rw->readers == 0){
    sem_post(&rw->writelock);
  }
  sem_post(&rw->lock);
}

void WriterLock(struct read_write_lock * rw)
{
  sem_wait(&rw->writelock);
}

void WriterUnlock(struct read_write_lock * rw)
{
  sem_post(&rw->writelock);
}
