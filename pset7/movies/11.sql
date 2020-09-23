SELECT movies.title
FROM movies
JOIN ratings ON ratings.movie_id = movies.id
WHERE movies.id
IN 	(
	SELECT stars.movie_id
	FROM stars
	WHERE stars.person_id
	IN (
		SELECT people.id
		FROM people
		WHERE people.name IS "Chadwick Boseman"
	)
)
ORDER BY ratings.rating DESC
LIMIT 5





