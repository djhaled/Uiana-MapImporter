using Newtonsoft.Json;
using Newtonsoft.Json.Serialization;

namespace cue4extractor
{
    class CustomResolver : DefaultContractResolver
    {
        private Dictionary<Type, JsonConverter> Converters { get; set; }

        public CustomResolver(Dictionary<Type, JsonConverter> converters)
        {
            Converters = converters;
        }

        protected override JsonObjectContract CreateObjectContract(Type objectType)
        {
            JsonObjectContract contract = base.CreateObjectContract(objectType);
            if (Converters.TryGetValue(objectType, out JsonConverter converter))
            {
                contract.Converter = converter;
            }
            return contract;
        }
    }
}
