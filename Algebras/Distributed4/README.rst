:author: Sebastian J. Bronner <sebastian@bronner.name>
:date: 2018-01-11

.. If desired, an HTML version of this file can be generated with the following
   command:

   rst2html5 README.rst > README.html

.. contents::

Linking to Secondo
==================

This algebra is intended to be compiled and used together with Secondo. The
general instructions for linking an algebra to Secondo [#link]_ apply with just
a small additional consideration: It requires that the library
``boost_filesystem`` be linked along with it.

Prerequisites for both the linking and compilation stages are that the library
and its header files are available. In Ubuntu 16.04 this is achieved by
installing the following packages:

* ``libboost-filesystem1.58-dev``
* ``libboost-filesystem1.58.0``
* ``libboost1.58-dev``

Finally, secondo has to be instructed how to link this algebra. Besides the
normal entries in ``Algebras/Management/AlgebraList.i.cfg`` and
``makefile.algebras``, an additional line is added to the latter. The complete
entry in ``makefile.algebras`` should be as follows::

  ALGEBRA_DIRS += Distributed4
  ALGEBRAS     += Distributed4Algebra
  COMMON_LD_FLAGS += -lboost_filesystem

Configuring Secondo Instances
=============================

This algebra facilitates a somewhat complex configuration of multiple Secondo
instances. It is therefore not intuitive how Secondo should best be configured
to be able to try it out. The following is a configuration suitable to that
purpose. It was created on the department of `Database Systems for New
Applications <http://dna.fernuni-hagen.de/>`_' ``newton`` cluster [#cluster]_.
It should be easy enough to adapt to any other environment, though.

Create the following files and directories on the named cluster servers:

``~/cfg/supervisor.ini`` [#home]_ on ``newton3`` [#n3]_:

  This is a copy of ``~/secondo/bin/SecondoConfig.ini`` with just a few lines
  changed as follows. Be sure to replace ``<username>`` with your own username
  Also **create the directory** named in ``SecondoHome``::

    SecondoHome=/home/<username>/dbs/supervisor
    GlobalMemory=1024
    #RTFlags += SMI:NoTransactions

  It is important that ``SMI:NoTransactions`` is **not** enabled because each
  SecondoMonitor will spawn several Secondo processes accessing a single
  database simultaneously.

``~/cfg/master.ini`` on ``newton3``:

  Same as ``supervisor.ini`` above, except that the two lines are changed as
  follows::

    SecondoHome=/home/<username>/dbs/master
    GlobalMemory=8192

``~/cfg/inserter.ini`` on ``newton{1,2,4,5}``:

  Same as ``supervisor.ini`` above, except that the two lines are changed as
  follows::

    SecondoHome=/home/<username>/dbs/inserter
    GlobalMemory=1024

``~/cfg/worker.ini`` on ``newton[1-5]``:

  Same as ``supervisor.ini`` above, except that the two lines are changed as
  follows::

    SecondoHome=/home/<username>/dbs/worker
    GlobalMemory=2048

``~/cfg/monitors.cfg`` [#ports]_ on ``newton3``::

  newton1 ~/cfg/inserter.ini      ~/secondo/bin   ~/dbs/inserter  10070
  newton2 ~/cfg/inserter.ini      ~/secondo/bin   ~/dbs/inserter  10070
  newton3 ~/cfg/master.ini        ~/secondo/bin   ~/dbs/master    10070
  newton4 ~/cfg/inserter.ini      ~/secondo/bin   ~/dbs/inserter  10070
  newton5 ~/cfg/inserter.ini      ~/secondo/bin   ~/dbs/inserter  10070
  newton1 ~/cfg/worker.ini        ~/secondo/bin   ~/dbs/worker    10071
  newton2 ~/cfg/worker.ini        ~/secondo/bin   ~/dbs/worker    10071
  newton3 ~/cfg/worker.ini        ~/secondo/bin   ~/dbs/worker    10071
  newton4 ~/cfg/worker.ini        ~/secondo/bin   ~/dbs/worker    10071
  newton5 ~/cfg/worker.ini        ~/secondo/bin   ~/dbs/worker    10071

Finally start up the Secondo Instances with either of the following commands::

  ~/secondo/bin/remoteMonitors ~/cfg/monitors.cfg start
  ~/secondo/bin/remoteMonitors2 ~/cfg/monitors.cfg start

The only difference is that ``remoteMonitors2`` has a somewhat more concise
output.

Preparing the Databases
=======================

Run the following commands on ``newton3`` to create and prepare a test database
named ``D4`` on all configured Secondo instances::

  cd ~/secondo/bin

Repeat the following sequence for every host/port combination specified in
``monitors.cfg`` above::

  ./SecondoTTYCS -h newton1 -p 10070

  create database d4;
  quit;

The remainder will be configured from the supervisor::

  ./SecondoTTYBDB -c ~/cfg/supervisor.ini

  create database d4;
  open database d4;
  let peers = [const rel(tuple([Host: string, Port: int, Config: string]))
    value (
      ("newton1.fernuni-hagen.de" 10070 "-")
      ("newton2.fernuni-hagen.de" 10070 "-")
      ("newton3.fernuni-hagen.de" 10070 "-")
      ("newton4.fernuni-hagen.de" 10070 "-")
      ("newton5.fernuni-hagen.de" 10070 "-")
    )];
  let workers = [const rel(tuple([Host: string, Port: int, Config: string]))
    value (
      ("newton1.fernuni-hagen.de" 10071 "-")
      ("newton2.fernuni-hagen.de" 10071 "-")
      ("newton3.fernuni-hagen.de" 10071 "-")
      ("newton4.fernuni-hagen.de" 10071 "-")
      ("newton5.fernuni-hagen.de" 10071 "-")
    )];
  let da = intstream(0,99) namedtransformstream[Id] extend[Value: .Id*1.1]
    ddistribute3["da", 10, FALSE, workers];
  query share("da", TRUE, peers);
  let dp = [const dpartition(Id: int)
    value (
      ((0 0) (10 1) (20 2) (30 3) (40 4) (50 5) (60 6) (70 7) (80 8) (90 9))
      da
    )];
  query share("dp", TRUE, peers);
  let ad = [const adist
    value (dp 1000 (
      ("newton1.fernuni-hagen.de" 10070 "-")
      ("newton2.fernuni-hagen.de" 10070 "-")
      ("newton3.fernuni-hagen.de" 10070 "-")
      ("newton4.fernuni-hagen.de" 10070 "-")
      ("newton5.fernuni-hagen.de" 10070 "-")
    ))];

Trying Out Some Operators
=========================

With the exception of the three lock operators, all operators implemented in
this algebra are intended for use on the supervisor and require an object of
type *adist* as an argument. They are designed to **redistribute** the records
stored in the underlying *darray* partitioned by the *dpartition*. Operators to
create, read, update, or delete records based on their value are not part of
this algebra. Such operators are expected to take the *dpartition* as an
argument. They should be operable on any of the peers defined in the *adist*, as
they all get a synchronized copy of the *dpartitian* and *darray*.

For this reason, all of these examples will be executed on the supervisor. It
might be interesting to look in on the workers or peers just to see how the
database objects have changed, though. That is completely up to you, and will
not be included in the steps outlined here [#debug]_::

  cd ~/secondo/bin
  ./SecondoTTYBDB -c ~/cfg/supervisor.ini

  open database d4;

If you look at the output of ``query da``, you will see that there are ten slots
arranged in standard order on the five workers. Standard order is defined in
*Distributed2Algebra* and means that the slots are assigned to workers round
robin. In our case that means the following:

* slot 0 -> worker 0 (``newton1:10071``)
* slot 1 -> worker 1 (``newton2:10071``)
* slot 2 -> worker 2 (``newton3:10071``)
* slot 3 -> worker 3 (``newton4:10071``)
* slot 4 -> worker 4 (``newton5:10071``)
* slot 5 -> worker 0
* slot 6 -> worker 1
* slot 7 -> worker 2
* slot 8 -> worker 3
* slot 9 -> worker 4

First we'll move slot 2 on worker 2 to worker 3::

  query moveslot(ad, 2, 3);

The effect of this can be seen on worker 2 (``da_2`` is missing), worker 3
(``da_2`` is now there), and the supervisor (``da``'s slot-to-worker map should
now be ``(0 1 3 3 4 0 1 2 3 4)``, indicating that both slots 2 and 3 are now on
worker 3).

Now let's split ``da_2`` into two slots::

  query splitslot(ad, 2);

The output from this query is ``10``. This is the slot number where half of the
values formerly in slot 2 were put. In addition to the slots it previously held,
worker 3 now also holds slot 10. ``da``'s map now contains an eleventh element
naming worker 3: ``(0 1 3 3 4 0 1 2 3 4 3)``. This time ``dp`` has changed, as
well. It now has 2 partitions for values between 20 and 30 split between slot 2
and slot 10: ``(20.0 2) (25.0 10)``.

The next logical attempt would be to merge two slots together. If we try to
merge slot 2 and slot 3, we would have a slot with a hole in it. Slot 2 contains
values between 20 and 25 and slot 3 values between 30 and 40. The values between
25 and 30 are in slot 10. Let's see what Secondo thinks about this attempt::

  query mergeslots(ad, 2, 3);

You should have received the message, ``Error: The partitions starting with
20.000000 and 30.000000 are not contiguous.`` The *Distributed4Algebra* doesn't
allow slots with holes in their mapped value range, as we would probably expect
intuitively.

So, let's merge slots 10 and 3 instead, to get a slot having values between 25
and 40::

  query mergeslots(ad, 10, 3);

The value ``3`` is returned. This indicates that the slot now holding the
combined data is slot 3. Slot 10 was removed from the system. (This happens
whenever the last slot in a *darray* is no longer needed.) ``dp``'s partitions
confirm the new value range for slot 3: ``(25.0 3)`` and ``da``'s map shows that
slot 10 is no more: ``(0 1 3 3 4 0 1 2 3 4)``.

Besides manipulating slots, this algebra also enables the removal and addition
of both workers and peers. Peers are any secondo instances that contain
synchronized copies of the *darray* and *dpartition* objects ``da`` and ``dp``.
They can operate on the workers independently. Usually these will be a master
and several inserters. *Distributed4Algebra* is not concerned with their
purpose, so they are simply called peers. Peers are recorded in the *adist*
object ``ad`` wihle workers are recorded in the *darray* object ``da`` (as known
from *Distributed2Algebra*).

Let's disconnect a worker first. Let's take worker 2::

  query removeworker(ad, 2);

Oops! Apparently, I was too quick about it: ``Error: The specified worker still
has slots mapped to it.`` Let's move the remaining slot on worker 2 to worker 0
and then remove worker 2. According to ``da``'s slot-to-worker map, the slot on
worker 2 is slot 7::

  query moveslot(ad, 7, 0);
  query removeworker(ad, 2);

This time it worked (return value ``TRUE``). Looking at ``da``'s worker list
now shows four workers. The former worker 2 (``newton3``) is missing. This also
means that the former worker 3 is now worker 2 and the former worker 4 is now
worker 3. The slot-to-worker map has taken these new worker numbers into
account: ``(0 1 2 2 3 0 1 0 2 3)``.

Now let's remove one of the peers. Let's pick peer 1 at random::

  query removepeer(ad, 1);

This operation shows a lot less communication with other Secondo instances. Only
the peer being removed needs to be talked to. Its copies of ``da`` and ``dp``
are removed as part of removing it from the local *adist* object ``ad``. No
other peers or any workers are involved. ``ad`` now shows a reduced list of
peers.

The operators ``moveslot``, ``removeworker``, and ``removepeer`` all took a
worker or a peer by number (list index). For convenience, they may also be
specified with hostname and port number as used in the worker or peer list. The
following commands would have had the same effect as the ones used above::

  query moveslot(ad, 2, "newton4.fernuni-hagen.de", 10071);
  query moveslot(ad, 7, "newton1.fernuni-hagen.de", 10071);
  query removeworker(ad, "newton3.fernuni-hagen.de", 10071);
  query removepeer(ad, "newton2.fernuni-hagen.de", 10070);

The operators ``addworker`` and ``addpeer`` are straightforward. To add the
worker and peer back that were just removed, you would use the following
commands::

  query addworker(ad, "newton3.fernuni-hagen.de", 10071, "-");
  query addpeer(ad, "newton2.fernuni-hagen.de", 10070, "-");

Those are all the operators needed to redistribute records in a *darray*.
However, three more operators were necessary to provide for synchronization.
Let's go through them one by one::

  query lock("da", TRUE);

This will lock the database object ``da`` for exclusive access. This is useful
when performing requests directly on the *darray* object underlying a
*dpartition* object, as those operators aren't aware of any concurrency and have
no concept of locking. Doing this will ensure that any of the above operators
and any future operators handling value-based access via the *dpartition* object
will not interfere during your request. This operator will wait forever to gain
the lock if it is already locked. The alternative operator will try to gain the
lock and fail with an error message if it isn't immediately possible::

  query trylock("da", TRUE);

There it is: ``Error: The mutex at
/dev/shm/secondo:home_bronner_dbs_governor_DEMO:da is already locked.`` The
second argument to these operators allows gaining sharable access. Let's have a
look at their behavior. But first, we have to unlock ``da``::

  query unlock("da");
  query lock("da", FALSE);
  query trylock("da", FALSE);

Now we hold 2 sharable access locks on ``da``. It is time to introduce a tricky
situation. Some of the above operators require an exclusive lock on ``da``.
Let's take ``splitslot`` as an example. If it is run while a lock on ``da`` is
held (as is currently the case), it will wait until that lock is released::

  query splitslot(ad, 0);

Fortunately, it provides an informational message to the user in this case:
``The mutex at /dev/shm/secondo:home_bronner_dbs_governor_DEMO:da is already
locked. Waiting for exclusive ownership.``

But now what? The lock won't be released automatically because it was manually
acquired. And the command line isn't available to unlock it. It would
theoretically be possible to log in to another Secondo instance and run unlock
from there, but there is a better solution: the companion utilitiy ``mutexset``
in ``Algebras/Distributed4/util``.  2 locks were acquired on ``da``, so at least
2 must be released before ``splitslot`` can proceed.

To use ``mutexset``, you must first compile it. It is not compiled with
Secondo. From a second shell on the same server as the Secondo instance
(``newton3``), run the following::

  make -C ~/secondo/Algebras/Distributed4/util

Then run the following as often as necessary until you see ``splitslot``
continue::

  ~/secondo/Algebras/Distributed4/util/mutexset /dev/shm/secondo\:<database-dir>_D4\:da unlock

That can be an invaluable tool if Secondo should ever crash while holding locks.
When such a left-over lock is detected later, it can be manually released
without resorting to killing the waiting Secondo instance.

Running ``mutexset`` without any arguments will provide usage information.

This has been an introduction by example of how the operators of the
*Distributed4Algebra* work. You should now be familiar enough with them to use
them for whatever purpose you wish.

----

.. [#link] `Programmer's Guide
   <http://dna.fernuni-hagen.de/Secondo.html/files/Documentation/Programming/ProgrammersGuide.pdf>`_.
   Section 1.7.3, p. 15 (PDF:19). Found at
   `<http://dna.fernuni-hagen.de/Secondo.html/content_docu_extend.html>`_
   (2018-01-10).

.. [#cluster] For more details about the ``newton`` cluster, see: `Distributed
   Query Processing in Secondo
   <http://dna.fernuni-hagen.de/Secondo.html/files/Documentation/General/DistributedQueryProcessinginSecondo.pdf>`_.
   Section 3.2, p. 6 (PDF:11). Found at
   `<http://dna.fernuni-hagen.de/Secondo.html/content_docu.html>`_
   (2018-01-10).

.. [#home] The symbol ``~`` stands for your personal home directory, usually
   ``/home/<username>``. It is understood by ``bash``, so the command ``mkdir
   ~/dbs`` is the same as ``mkdir /home/<username>/dbs``.

.. [#n3] Just to avoid having ``newton1`` used by everyone as the primary (and
   therefore most loaded) member of the cluster, these instructions use
   ``newton3`` as the central server.

.. [#ports] The port numbers chosen here are composed from my ``uid`` (command
   `id`) and an additional digit. This helps to prevent conflicts arising from
   other users' processes listening on the same port.

.. [#debug] If you are interested in seeing the communication happening between
   Secondo instances, you can run either ``query traceCommands(TRUE);`` or
   ``query da2enableLog(TRUE);`` or both. To see the log from ``da2enableLog``,
   run ``query da2Log() consume;``.
