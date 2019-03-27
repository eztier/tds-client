
if object_id('tempdb..#c') is not null drop table #c;
create table #c(name varchar(120), score int); 

-- insert #c values ('a',1), ('b',2), ('c',3);

declare c cursor local for select * from #c;
declare @name varchar(120), @score int;

open c;
-- dblib does not support server side cursor fetch yet.
-- fetch next from c into @name, @int;

-- while @@fetch_status = 0 begin

-- fetch next from c into @name, @int;
-- end

close c;
deallocate c;

declare @cnt int = 0;
while @cnt < 10 begin

insert into #c
select char(@cnt+97), @cnt;

select @cnt = @cnt + 1;
end


select * into #d from #c;

delete #d where name = 'b';

select * from #d;

