use `telepath`;
drop procedure if exists `get_recommended_request_ids`;

DELIMITER $$

use `telepath`$$
create definer=`root`@`%` procedure `get_recommended_request_ids`(in selector_rid bigint(20) unsigned)
    comment 'Returns recommended request IDs by the selector argument.'
begin
	/*
	Returns recommended request IDs following the selector argument, selector request ID.
	The result rows ordered by similarity to selector - better similarity first.
	The result table includes two columns:
		RID: for recommended request ID.
		cosin_similarity: the cosin similarity to the selector request ID -
			real number in range [0..1], where 1 represents best similarity.
	Search domain limitid hardcoded by 100000.
	*/
	drop table if exists tmp_relevant_attributes;
	create temporary table if not exists tmp_relevant_attributes
		select att_id, attribute_score_normal
		from attribute_scores
		where RID=selector_rid and attribute_scores.attribute_score_normal > 0;

	do (select @selector_vector_norm := sqrt(sum(pow(tmp_relevant_attributes.attribute_score_normal, 2)))
			as selector_vector_norm
		from tmp_relevant_attributes);

	select scores.RID,
	  sum(scores.attribute_score_normal * tmp_relevant_attributes.attribute_score_normal) /
	  (sqrt(sum(pow(scores.attribute_score_normal, 2))) * @selector_vector_norm) cosin_similarity
	from (select RID, att_id, attribute_score_normal from attribute_scores limit 10000) scores
	  left join tmp_relevant_attributes on scores.att_id = tmp_relevant_attributes.att_id
	group by scores.RID
	order by cosin_similarity desc
	limit 50;
	
	drop table if exists tmp_relevant_attributes;
end$$

DELIMITER ;
