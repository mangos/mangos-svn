# Copyright (C) 2005,2006 MaNGOS <http://www.mangosproject.org/>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

## Process this file with automake to produce Makefile.in

EXTRA_DIST = reconf configure
SUBDIRS = doc dep src sql

# Win32 project workspace for Visual Studio .NET 2003
EXTRA_DIST += \
	win/mangosdVC71.sln \
	win/VC71/framework.vcproj \
	win/VC71/game.vcproj \
	win/VC71/mangosd.vcproj \
	win/VC71/realmd.vcproj \
	win/VC71/shared.vcproj \
	win/VC71/zlib.vcproj \
	win/VC71/zthread.vcproj

# Win32 project workspace for Visual Studio .NET 2005
EXTRA_DIST += \
	win/mangosdVC80.sln \
	win/VC80/framework.vcproj \
	win/VC80/game.vcproj \
	win/VC80/mangosd.vcproj \
	win/VC80/realmd.vcproj \
	win/VC80/shared.vcproj \
	win/VC80/zlib.vcproj \
	win/VC80/zthread.vcproj

# Win32 project workspace for Visual Studio .NET 2003 and Intel Compiler
EXTRA_DIST += \
	win_intel/mangosdVC71.sln \
	win_intel/VC71/framework.vcproj \
	win_intel/VC71/game.icproj \
	win_intel/VC71/game.vcproj \
	win_intel/VC71/mangosd.icproj \
	win_intel/VC71/mangosd.vcproj \
	win_intel/VC71/realmd.icproj \
	win_intel/VC71/realmd.vcproj \
	win_intel/VC71/script.icproj \
	win_intel/VC71/script.vcproj \
	win_intel/VC71/shared.icproj \
	win_intel/VC71/shared.vcproj \
	win_intel/VC71/zlib.icproj \
	win_intel/VC71/zlib.vcproj \
	win_intel/VC71/zthread.icproj \
	win_intel/VC71/zthread.vcproj

# MySQL headers
EXTRA_DIST += \
	dep/include/mysql/config-win.h \
	dep/include/mysql/dbug.h \
	dep/include/mysql/errmsg.h \
	dep/include/mysql/libmysqld.def \
	dep/include/mysql/Libmysql.def \
	dep/include/mysql/m_ctype.h \
	dep/include/mysql/m_string.h \
	dep/include/mysql/my_alloc.h \
	dep/include/mysql/my_getopt.h \
	dep/include/mysql/my_global.h \
	dep/include/mysql/my_list.h \
	dep/include/mysql/my_pthread.h \
	dep/include/mysql/mysql_com.h \
	dep/include/mysql/mysqld_error.h \
	dep/include/mysql/mysql.h \
	dep/include/mysql/mysql_version.h \
	dep/include/mysql/my_sys.h \
	dep/include/mysql/raid.h

# OpenSSL headers
EXTRA_DIST += \
	dep/include/openssl/aes.h \
	dep/include/openssl/asn1.h \
	dep/include/openssl/asn1_mac.h \
	dep/include/openssl/asn1t.h \
	dep/include/openssl/bio.h \
	dep/include/openssl/blowfish.h \
	dep/include/openssl/bn.h \
	dep/include/openssl/buffer.h \
	dep/include/openssl/cast.h \
	dep/include/openssl/comp.h \
	dep/include/openssl/conf_api.h \
	dep/include/openssl/conf.h \
	dep/include/openssl/crypto.h \
	dep/include/openssl/des.h \
	dep/include/openssl/des_old.h \
	dep/include/openssl/dh.h \
	dep/include/openssl/dsa.h \
	dep/include/openssl/dso.h \
	dep/include/openssl/ebcdic.h \
	dep/include/openssl/ec.h \
	dep/include/openssl/engine.h \
	dep/include/openssl/e_os2.h \
	dep/include/openssl/err.h \
	dep/include/openssl/evp.h \
	dep/include/openssl/hmac.h \
	dep/include/openssl/idea.h \
	dep/include/openssl/krb5_asn.h \
	dep/include/openssl/kssl.h \
	dep/include/openssl/lhash.h \
	dep/include/openssl/md2.h \
	dep/include/openssl/md4.h \
	dep/include/openssl/md5.h \
	dep/include/openssl/mdc2.h \
	dep/include/openssl/objects.h \
	dep/include/openssl/obj_mac.h \
	dep/include/openssl/ocsp.h \
	dep/include/openssl/opensslconf.h \
	dep/include/openssl/opensslv.h \
	dep/include/openssl/ossl_typ.h \
	dep/include/openssl/pem2.h \
	dep/include/openssl/pem.h \
	dep/include/openssl/pkcs12.h \
	dep/include/openssl/pkcs7.h \
	dep/include/openssl/rand.h \
	dep/include/openssl/rc2.h \
	dep/include/openssl/rc4.h \
	dep/include/openssl/rc5.h \
	dep/include/openssl/ripemd.h \
	dep/include/openssl/rsa.h \
	dep/include/openssl/safestack.h \
	dep/include/openssl/sha.h \
	dep/include/openssl/ssl23.h \
	dep/include/openssl/ssl2.h \
	dep/include/openssl/ssl3.h \
	dep/include/openssl/ssl.h \
	dep/include/openssl/stack.h \
	dep/include/openssl/symhacks.h \
	dep/include/openssl/tls1.h \
	dep/include/openssl/tmdiff.h \
	dep/include/openssl/txt_db.h \
	dep/include/openssl/ui_compat.h \
	dep/include/openssl/ui.h \
	dep/include/openssl/x509.h \
	dep/include/openssl/x509v3.h \
	dep/include/openssl/x509_vfy.h

# SQLite headers
EXTRA_DIST += \
	dep/include/sqlite/sqlite.h

# zlib headers
EXTRA_DIST += \
	dep/include/zlib/zconf.h \
	dep/include/zlib/zlib.h

# ZThread headers
EXTRA_DIST += \
	dep/include/zthread/AtomicCount.h \
	dep/include/zthread/Barrier.h \
	dep/include/zthread/BiasedReadWriteLock.h \
	dep/include/zthread/BlockingQueue.h \
	dep/include/zthread/BoundedQueue.h \
	dep/include/zthread/Cancelable.h \
	dep/include/zthread/ClassLockable.h \
	dep/include/zthread/ConcurrentExecutor.h \
	dep/include/zthread/Condition.h \
	dep/include/zthread/Config.h \
	dep/include/zthread/CountedPtr.h \
	dep/include/zthread/CountingSemaphore.h \
	dep/include/zthread/Exceptions.h \
	dep/include/zthread/Executor.h \
	dep/include/zthread/FairReadWriteLock.h \
	dep/include/zthread/FastMutex.h \
	dep/include/zthread/FastRecursiveMutex.h \
	dep/include/zthread/GuardedClass.h \
	dep/include/zthread/Guard.h \
	dep/include/zthread/Lockable.h \
	dep/include/zthread/LockedQueue.h \
	dep/include/zthread/MonitoredQueue.h \
	dep/include/zthread/Mutex.h \
	dep/include/zthread/NonCopyable.h \
	dep/include/zthread/PoolExecutor.h \
	dep/include/zthread/PriorityCondition.h \
	dep/include/zthread/Priority.h \
	dep/include/zthread/PriorityInheritanceMutex.h \
	dep/include/zthread/PriorityMutex.h \
	dep/include/zthread/PrioritySemaphore.h \
	dep/include/zthread/Queue.h \
	dep/include/zthread/ReadWriteLock.h \
	dep/include/zthread/RecursiveMutex.h \
	dep/include/zthread/Runnable.h \
	dep/include/zthread/Semaphore.h \
	dep/include/zthread/Singleton.h \
	dep/include/zthread/SynchronousExecutor.h \
	dep/include/zthread/Task.h \
	dep/include/zthread/ThreadedExecutor.h \
	dep/include/zthread/Thread.h \
	dep/include/zthread/ThreadLocal.h \
	dep/include/zthread/ThreadLocalImpl.h \
	dep/include/zthread/Time.h \
	dep/include/zthread/Waitable.h \
	dep/include/zthread/ZThread.h

# MySQL shared library (Win32)
EXTRA_DIST += \
	dep/lib/debug/libmySQL.dll \
	dep/lib/debug/libmySQL.lib \
	dep/lib/release/libmySQL.dll \
	dep/lib/release/libmySQL.lib

# OpenSSL shared library (Win32)
EXTRA_DIST += \
	dep/lib/debug/libeay32.dll \
	dep/lib/debug/libeay32.lib \
	dep/lib/release/libeay32.dll \
	dep/lib/release/libeay32.lib

# SQLite shared library (Win32)
EXTRA_DIST += \
	dep/lib/debug/sqlite.lib \
	dep/lib/release/sqlite.lib
