# Quick connection to departmental servers 

To log in to `granger1` from your local machine, you could manually connect to `portal` and then `granger1` as described ([here](https://www.cs.virginia.edu/wiki/doku.php?id=linux_ssh_access)). 

### Connect to server using one line of command

```
$ ssh -l USERNAME granger1 -J portal.cs.virgina.edu
```
The -J option is available with your local ssh client OpenSSH >= 7.3p1. See [here](https://unix.stackexchange.com/questions/423205/can-i-access-ssh-server-by-using-another-ssh-server-as-intermediary/423211#423211) for more details. For instance, my version: 

```
$ ssh -V
OpenSSH_7.6p1 Ubuntu-4ubuntu0.3, OpenSSL 1.0.2n  7 Dec 2017
```

### Avoid typing password every time 

Append your local pubkey (~/.ssh/id_rsa.pub) to both portal and granger1 (~/.ssh/authorized_keys)

### Further shorten the ssh command  

Append to your local file (~/.ssh/config). **Replace USRENAME with your actual username**: 

```
Host granger1
   User USERNAME
   HostName granger1.cs.virginia.edu
   ProxyJump USERNAME@portal.cs.virginia.edu:22
```
With the configuration, your local ssh client knows that when connecting  to host `granger1`, use `portal` as the jump proxy. So you can directly connect to `granger1` from your local machine: 
```
$ ssh granger1
Welcome to Ubuntu 20.04 LTS (GNU/Linux 5.4.0-45-generic x86_64)
...
```

