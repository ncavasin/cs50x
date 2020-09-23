SELECT people.name
FROM people
WHERE people.id
IN (
	SELECT stars.person_id
	FROM stars
	JOIN movies ON movies.id = stars.movie_id
	WHERE movies.year = 2004
	)
ORDER BY people.birth;

