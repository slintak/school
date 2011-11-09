BEGIN {
	N = 10000;
	print 2
	print N, " ", N
	for (i = 0; i < N; i++) {
		for (j = 0; j < N; j++)
			printf("%d ", int(rand() * 10));
		print "\n";
	}
}
