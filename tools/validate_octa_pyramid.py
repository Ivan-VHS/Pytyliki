#!/usr/bin/env python3
"""Validate the four-project OCTA-8 federation."""
from __future__ import annotations
import argparse,json,sys
from pathlib import Path
from typing import Any
ROOT=Path(__file__).resolve().parents[1];T=ROOT/"federation/octa-pyramid.v1.json";S=ROOT/"schemas/octa-pyramid.v1.schema.json";P=ROOT/"project-contract.json"
PROJECTS={"reaper","pytyliki","oplague","ventusample"};FACETS={"facet.identity","facet.contracts","facet.transport","facet.provenance","facet.validation","facet.privacy","facet.safety","facet.audit"};STATUSES={"implemented","partial","planned","not_applicable"}
EDGES={
("reaper","oplague","json-file","reaper.oplage-comparison.v1","artifact","implemented"),("oplague","reaper","json-file","oplage.device-snapshot.v1","artifact","implemented"),
("reaper","pytyliki","native-ffi","pytyliki.c-abi.v1","request","implemented"),("pytyliki","reaper","native-ffi","pytyliki.c-abi.v1","response","implemented"),
("oplague","pytyliki","native-ffi","pytyliki.c-abi.v1","request","implemented"),("pytyliki","oplague","native-ffi","pytyliki.c-abi.v1","response","implemented"),
("ventusample","reaper","json-file","ventusample.pack.v1","artifact","partial"),("reaper","ventusample","json-file","ventusample.render-request.v1","artifact","planned"),
("ventusample","pytyliki","native-ffi","pytyliki.c-abi.v1","request","planned"),("pytyliki","ventusample","native-ffi","pytyliki.c-abi.v1","response","planned"),
("ventusample","oplague","none","not-applicable.v1","none","not_applicable"),("oplague","ventusample","none","not-applicable.v1","none","not_applicable")}
def load(p:Path):
 def unique(pairs):
  d={}
  for k,v in pairs:
   if k in d:raise ValueError(f"duplicate JSON key: {k}")
   d[k]=v
  return d
 try:return json.loads(p.read_text(encoding="utf-8"),object_pairs_hook=unique,parse_constant=lambda t:(_ for _ in ()).throw(ValueError(f"non-finite JSON: {t}")))
 except OSError as e:raise ValueError(f"cannot read {p}: {e}") from e
def need(ok,msg):
 if not ok:raise ValueError(msg)
def validate():
 s=load(S);need(s.get("$schema")=="https://json-schema.org/draft/2020-12/schema" and s.get("title")=="OCTA-8 Four-Project Federation Pyramid v1","invalid schema")
 x=load(T);need(x.get("schema")=="octa.federation-pyramid.v1" and x.get("topology_id")=="octa-8","invalid topology")
 sh=x["shape"];need(sh.get("kind")=="octagonal-pyramid" and sh.get("base_sides")==8 and sh.get("triangular_facets")==8 and sh.get("apex_vertex")=="octa-hub","invalid geometry")
 apex=x["apex"];need(apex.get("executable") is False and apex.get("single_point_of_failure") is False,"apex must be non-executable")
 fs=x["facets"];ids=[v["id"] for v in fs];need(len(fs)==8 and len(ids)==len(set(ids)) and set(ids)==FACETS and apex.get("linked_facets")==ids and sorted(v["side"] for v in fs)==list(range(1,9)),"invalid facets")
 base=x["base_mesh"];need(base["project_ids"]==["reaper","pytyliki","oplague","ventusample"] and base["directed_peer_links"]==12 and base["self_loops"]==4,"invalid peer mesh")
 ls=x["peer_links"];actual={(v["from"],v["to"],v["transport"],v["interface"],v["flow"],v["status"]) for v in ls};need(len(ls)==12 and actual==EDGES and len({v["id"] for v in ls})==12,"peer links/status differ from canonical 12")
 loops=x["self_loops"];need(len(loops)==4 and {v["project"] for v in loops}==PROJECTS,"four self-loops required")
 sem=x["semantics"];need(sem.get("complete_pairwise_product") is True and sem.get("directed_link_count_formula")=="n*(n-1)" and sem.get("hub_is_not_a_runtime_gateway") is True and sem.get("request_response_edges_are_synchronous") is True and sem.get("no_implicit_native_loading") is True and sem.get("no_driver_mutation_through_mesh") is True and sem.get("producer_export_does_not_imply_consumer_integration") is True,"invalid federation semantics")
 p=load(P)["project"]["id"];need(p in PROJECTS and p in base["project_ids"],"local project is missing from the mesh")
 return {"valid":True,"topology":"octa-8","project":p,"facets":8,"directed_peer_links":12,"self_loops":4}
def main():
 a=argparse.ArgumentParser(description=__doc__);a.add_argument("--json",action="store_true");o=a.parse_args()
 try:r=validate()
 except (ValueError,TypeError,KeyError) as e:print(f"OCTA-8 INVALID: {e}",file=sys.stderr);return 1
 print(json.dumps(r,ensure_ascii=False,indent=2,sort_keys=True) if o.json else f"OCTA-8 valid | {r['project']} | 8 facets | 12 directed links | 4 self-loops");return 0
if __name__=="__main__":raise SystemExit(main())
