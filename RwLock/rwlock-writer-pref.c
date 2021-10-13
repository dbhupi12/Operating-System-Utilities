#include "rwlock.h"

void InitalizeReadWriteLock(struct read_write_lock * rw)
{
  rw->readers = 0;
  sem_init(&rw->lock,0,1);
  sem_init(&rw->writelock,0,1);
  sem_init(&rw->read_check,0,1);
  sem_init(&rw->write_check,0,1);
  rw->writers = 0;
}

void ReaderLock(struct read_write_lock * rw)
{
  sem_wait(&rw->read_check);
  sem_wait(&rw->lock);
  rw->readers++;
  if(rw->readers == 1){
    sem_wait(&rw->write_check);
  }
  sem_post(&rw->lock);
  sem_post(&rw->read_check);
}

void ReaderUnlock(struct read_write_lock * rw)
{
  sem_wait(&rw->lock);
  rw->readers--;
  if(rw->readers == 0){
    sem_post(&rw->write_check);
  }
  sem_post(&rw->lock);
}

void WriterLock(struct read_write_lock * rw)
{
  sem_wait(&rw->writelock);
  rw->writers++;
  if(rw->writers == 1){
    sem_wait(&rw->read_check);
  }
  sem_post(&rw->writelock);
  sem_wait(&rw->write_check);
}

void WriterUnlock(struct read_write_lock * rw)
{
 sem_post(&rw->write_check);
 sem_wait(&rw->writelock);
 rw->writers--;
 if(rw->writers == 0){
   sem_post(&rw->read_check);
 }
 sem_post(&rw->writelock);
}
