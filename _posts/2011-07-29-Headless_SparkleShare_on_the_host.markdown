---
layout: post
title: Headless SparkleShare on the host
---

This post describes how to efficiently synchronize a SparkleShare project with a folder on the host.

Use-case
========

I discovered [SparkleShare](http://sparkleshare.org/) today and decided to use it to replace [Ubuntu One](https://one.ubuntu.com/), in a quixotic quest of having total control over my cloud one day. One of my uses of Ubuntu One is for synchronizing some configuration files (`~/.bash_alias`, `~/Templates`, etc.) in a common folder (say, `configs`). Link to these files replace the actual files. e.g. :

{% highlight sh %}
ln -s ~/configs/.bash_alias ~/.bash_alias
ln -s ~/configs/Templates ~/Templates
{% endhighlight %}


My adventures in finding a solution
===================================

I use my own server as a host of my SparkleShare projects, and I want to use my configuration files on this server. I started to search for some headless SparkleShare daemon that would synchronize the `configs` folder with the git repository that SparkleShare uses as a synchronization hub. Apparently there is a headless version of SparkleShare, but it doesn't seem very clean, so I started playing with the git repository. Puzzled by the `--bare` option, I found a post explaining [how to auto-publish a website using git](http://sitaramc.github.com/tips/auto-publish.html). It provided me with a solution to my problem, in combination with the [instructions to set up my own server from SparkleShare](https://github.com/hbons/SparkleShare/wiki/How-to-set-up-your-own-server).

Example
=======

{% highlight sh %}
#create a new SparkleShare project:
git init --bare configs.git
cd configs.git
#tell git that the repository is not just a repository
git config core.bare false
#tell git where to extract the working copy
git config core.worktree ~/configs
#tell git not to complain about this apparently bad practice
git config receive.denyCurrentBranch ignore
#tell git to update the working copy
#each time the repository receives a new commit
cat > hooks/post-receive <<EOF
#!/bin/sh
git checkout -f
EOF
chmod +x hooks/post-receive
{% endhighlight %}


Conclusion
==========

That's it! Now the file you put from other machines in the configs project will be created/updated in the `~/configs` folder on the host.

PS: if you're an experienced git user and you're having a heart attack, it's probably due to my lack of git terminology and/or git good practice. As an excuse, it is the first time I go beyond git clone, but you will probably recognize the mark of an evil bzr user ;)... So I would be happy to get some constructive feedback on my git usage, or maybe on the SparkleShare feature that I missed and that does exactly what this post describes, only better...

