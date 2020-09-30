/* vim: set et sw=3 sts=3: */

/*
 *  fini - minimal PID 1 / process reaper for Linux containers.
 *
 *                                 ..   M M     
 *                          . :MMMO, ...77M     
 *                       =MM ...       . ~M     
 *                   .M8..               7$     
 *              . .MZ..        ..   ..  .I.:    
 *              M8,.     ...OMMMM=:...,.:N?M    
 *            M=I.   .~MM,..             M7M    
 *         .M=.    MM ~MMMMMMMMM         M~I    
 *         M. ..M:  .MMMMMMMMMMMMM       M7M..  
 *       .M .=M    $MMMMMMMMMMMMMMM      .MM=.  
 *       M.MD.    MMMMMMIII77777MMMM.     MMM.  
 *      M:M.     .MMMD7I7  .     IIMMM    MMM   
 *      7.       MMMM77I.   ..    ?MMM    MMM   
 *               MMMN77I  . I.... 7ZMM    $MM   
 *               MMMI77 IMI 7.I77 7DMM     MM   
 *               MMM7I .MMM.. MMZ. $MM.    MMM  
 *               MMM7IMMMMM   MMMM IMM.    MMM  
 *              .MMMIMMMMM    IMMMM.7MM    MMM  
 *               MM7.MMM:.      MMM. MM    MMM. 
 *              .7MI 7.     ~ . .7I  MM7    MM. 
 *              .7MI.7I   7ZMI   'I 7MMM   .MMI 
 *               .MM~I~         ..7IMMMM    MMM 
 *                MMM7 MMMZI~~IM7I.MMMMMZ.  MMM 
 *              .,MMM 77MMMMMMMM' 7MMMMMM   MMM 
 *               ZMMMI. MMMMMMM.: MMMMMMM   MMM.
 *              .7MMMM   MMMMM.  IMMMMMMMM, =MM:
 *               MMMMMM      .  7MMMMMMMMMMMMMMM
 *           MMMMMMMMMMM   . .  MMMMMMMMMMMMMMMM
 *       MMMMMMMMMMMMMMMM      MMMMMMMMMMMMMMMMM
 *      MMMMMMMMMMMMMMMMMI.   .MMMMMMMMMMMMMMMMM
 *      MMMMMMMMMMMMMMMMMM    IMMMMMMMMMMMMMMMMM
 *      MMMMMMMMMMMMMMMMMM~  .MMMMMMMMMMMMMMMMMM
 *      MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
 *
 * Copyright (C) 2020 Jānis Rukšāns < thedogfarted at alphabet's
 * webmail service >
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; version 2 of
 * the License.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * For the full text of the License, please see COPYING.
 *
 * The ASCII art Reaper was created by a computer program; I
 * don't think computer programs can be copyright holders, or
 * have any other rights thereof.
 */

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <signal.h>
#include <errno.h>

#include <string.h>
#include <stdio.h>

#ifndef  WAIT_TIMEOUT
#define  WAIT_TIMEOUT 10
#endif


static void
write_stderr(const char *msg) {
   write(STDERR_FILENO, msg, strlen(msg));
}


static volatile
sig_atomic_t exiting = 0;

static void
sighandler(int signum) {
   if (signum == SIGALRM) {
      write_stderr("fini: Forced exit, killing remaining processes\n");
      _exit(1);
   }

   if (signum == SIGINT)
      write_stderr("fini: Received SIGINT\n");
   else
   if (signum == SIGTERM)
      write_stderr("fini: Received SIGTERM\n");

   if ((signum == SIGTERM || signum == SIGINT) && !exiting) {
      exiting = 1;
      alarm(WAIT_TIMEOUT);
   }
   kill(-1, signum);
}


int
main() {
   int exit_status = 0;
   struct sigaction sa = { };

   if (getpid() != 1)
      return 1;

   sa.sa_handler = sighandler;
   sigaction(SIGTERM, &sa, NULL);
   sigaction(SIGINT,  &sa, NULL);
   sigaction(SIGALRM, &sa, NULL);

   while (1) {
      int status, exit_code = 0;
      pid_t pid;
      
      pid = wait(&status);
      if (pid == -1) {
         if (errno == ECHILD)
            _exit(exit_status);
         continue;
      }

      if (WIFEXITED(status)) {
         fprintf(stderr, "fini: PID %ld exited with status %d\n",
            (long) pid, WEXITSTATUS(status));
         exit_code = WEXITSTATUS(status);
      } else
      if (WIFSIGNALED(status)) {
         fprintf(stderr, "fini: PID %ld was killed by signal %d\n",
            (long) pid, WTERMSIG(status));
         exit_code = 128 + WTERMSIG(status);
      }
      if (!exit_status) {
         exit_status = exit_code;
      }

      if (!exiting) {
         fprintf(stderr, "fini: Sending SIGTERM to all processes\n");
         exiting = 1;
         alarm(WAIT_TIMEOUT);
         kill(-1, SIGTERM);
      }
   }

   return 0; /* not reached */
}
