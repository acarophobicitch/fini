
fini - minimal PID 1 / process reaper for Linux containers
==========================================================


                                 ..   M M     
                          . :MMMO, ...77M     
                       =MM ...       . ~M     
                   .M8..               7$     
              . .MZ..        ..   ..  .I.:    
              M8,.     ...OMMMM=:...,.:N?M    
            M=I.   .~MM,..             M7M    
         .M=.    MM ~MMMMMMMMM         M~I    
         M. ..M:  .MMMMMMMMMMMMM       M7M..  
       .M .=M    $MMMMMMMMMMMMMMM      .MM=.  
       M.MD.    MMMMMMIII77777MMMM.     MMM.  
      M:M.     .MMMD7I7  .     IIMMM    MMM   
      7.       MMMM77I.   ..    ?MMM    MMM   
               MMMN77I  . I.... 7ZMM    $MM   
               MMMI77 IMI 7.I77 7DMM     MM   
               MMM7I .MMM.. MMZ. $MM.    MMM  
               MMM7IMMMMM   MMMM IMM.    MMM  
              .MMMIMMMMM    IMMMM.7MM    MMM  
               MM7.MMM:.      MMM. MM    MMM. 
              .7MI 7.     ~ . .7I  MM7    MM. 
              .7MI.7I   7ZMI   'I 7MMM   .MMI 
               .MM~I~         ..7IMMMM    MMM 
                MMM7 MMMZI~~IM7I.MMMMMZ.  MMM 
              .,MMM 77MMMMMMMM' 7MMMMMM   MMM 
               ZMMMI. MMMMMMM.: MMMMMMM   MMM.
              .7MMMM   MMMMM.  IMMMMMMMM, =MM:
               MMMMMM      .  7MMMMMMMMMMMMMMM
           MMMMMMMMMMM   . .  MMMMMMMMMMMMMMMM
       MMMMMMMMMMMMMMMM      MMMMMMMMMMMMMMMMM
      MMMMMMMMMMMMMMMMMI.   .MMMMMMMMMMMMMMMMM
      MMMMMMMMMMMMMMMMMM    IMMMMMMMMMMMMMMMMM
      MMMMMMMMMMMMMMMMMM~  .MMMMMMMMMMMMMMMMMM
      MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM


`fini` is a tiny PID 1 / process reaper for Linux containers.

Unlike other minimalist inits / PID 1s, `fini` does not start any
child processes at all.  Instead it assumes that all processes
already have been started, and just waits for them to exit while
forwarding some signals (currently `SIGINT` and `SIGTERM`).

If `fini` detects that one of the processes has exited, it will
send `SIGTERM` to the remaining processes, effectively shutting
down (or restarting) the container.  This is intentional, see
below for the motivation behind `fini`.  However, this also means
that `fini` cannot be used for reaping zombies;  all processes
running in the container must properly wait for their children.


Using fini
----------

To use `fini`, add it to your container, start whatever services
you need, and `exec` `fini` as the last step to replace the
current PID 1.  Example using a shell script as the entrypoint:

```sh
#!/bin/sh

my-cool-service --daemonize
my-other-service &

exec /sbin/fini
```

Hence the name _fini_ - rather than initializing the container,
`fini` makes sure it is stopped properly.


Why another PID 1?
------------------

`fini` was created to scratch an itch - to run multiple services
in a single container, exiting the container when _any_ one of
them dies.

Existing minimal inits like [Tini][1] or [dumb-init][2] help with
signal handling, but I couldn't find a simple way to meet the
exit requirement.  First I tried to implement it in the same
shell script used to start the services, but shell has its own
quirks to deal with.  After briefly considering handing over
the task of being the PID 1 to one of the services, an idea was
born to use a dedicated program for it, and with that - `fini`.


Acknowledgements
----------------

`fini` was inspired by Tini, dumb-init, [suckless init][3] and,
perhaps most importantly, Rich Felker's [minimal init][4].


[1]: https://github.com/krallin/tini
[2]: https://github.com/Yelp/dumb-init
[3]: https://git.suckless.org/sinit/
[4]: http://ewontfix.com/14/
