//
// k8s_namespace_handler.cpp
//

#include "k8s_namespace_handler.h"
#include "sinsp.h"
#include "sinsp_int.h"

// filters normalize state and event JSONs, so they can be processed generically:
// event is turned into a single-entry array, state is turned into an array of ADDED events

std::string k8s_namespace_handler::EVENT_FILTER =
	"{"
	" type: .type,"
	" apiVersion: .object.apiVersion,"
	" kind: .object.kind,"
	" items:"
	" [ .object |"
	"  {"
	"   name: .metadata.name,"
	"   uid: .metadata.uid,"
	"   timestamp: .metadata.creationTimestamp,"
	"   labels: .metadata.labels"
	"  }"
	" ]"
	"}";

std::string k8s_namespace_handler::STATE_FILTER =
	"{"
	" type: \"ADDED\","
	" apiVersion: .apiVersion,"
	" kind: \"Namespace\","
	" items:"
	" ["
	"  .items[] |"
	"  {"
	"   name: .metadata.name,"
	"   uid: .metadata.uid,"
	"   timestamp: .metadata.creationTimestamp,"
	"   labels: .metadata.labels"
	"  }"
	" ]"
	"}";

k8s_namespace_handler::k8s_namespace_handler(k8s_state_t& state,
	collector_t& collector,
	std::string url,
	const std::string& http_version,
	ssl_ptr_t ssl,
	bt_ptr_t bt):
		k8s_handler(collector, "k8s_namespace_handler", url, "/api/v1/namespaces",
					STATE_FILTER, EVENT_FILTER, http_version,
					1000L, ssl, bt, &state)
{
}

k8s_namespace_handler::~k8s_namespace_handler()
{
}

void k8s_namespace_handler::handle_component(const Json::Value& json, const msg_data* data)
{
	if(data)
	{
		if(m_state)
		{
			k8s_ns_t& ns =
				m_state->get_component<k8s_namespaces, k8s_ns_t>(m_state->get_namespaces(),
																 data->m_name, data->m_uid);
			k8s_pair_list entries = k8s_component::extract_object(json, "labels");
			if(entries.size() > 0)
			{
				ns.set_labels(std::move(entries));
			}
		}
		else
		{
			throw sinsp_exception("K8s node handler: state is null.");
		}
	}
	else
	{
		throw sinsp_exception("K8s namespace handler: data is null.");
	}
}