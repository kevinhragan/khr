#!/bin/sh

##
# Write down the current transcription queue stats
##
CREATE_DATA=`mysql -s -u gvuser -pgotvoice -D gv <<EOF
create temporary table tmp_stats (
  \\\`new\\\` int(11) NOT NULL default 0,
  \\\`assigned\\\` int(11) NOT NULL default 0,
  \\\`complete\\\` int(11) NOT NULL default 0,
  \\\`deleted\\\` int(11) NOT NULL default 0,
  \\\`timeout\\\` int(11) NOT NULL default 0,
  \\\`timeout-complete\\\` int(11) NOT NULL default 0
);

insert into tmp_stats (\\\`new\\\`) values (0);
update tmp_stats set \\\`new\\\` = (select count(*) from transcription_queue where status = 'new');
update tmp_stats set \\\`assigned\\\` = (select count(*) from transcription_queue where status = 'assigned');
update tmp_stats set \\\`complete\\\` = (select count(*) from transcription_queue where status = 'complete');
update tmp_stats set \\\`deleted\\\` = (select count(*) from transcription_queue where status = 'deleted');
update tmp_stats set \\\`timeout\\\` = (select count(*) from transcription_queue where status = 'timeout');
update tmp_stats set \\\`timeout-complete\\\` = (select count(*) from transcription_queue where status = 'timeout-complete');

insert into transcription_queue_stats (\\\`new\\\`, assigned, complete, deleted, timeout, \\\`timeout-complete\\\`)
select tmp_stats.\\\`new\\\`, tmp_stats.assigned, tmp_stats.complete, tmp_stats.deleted, tmp_stats.timeout, tmp_stats.\\\`timeout-complete\\\` from tmp_stats;
EOF`
